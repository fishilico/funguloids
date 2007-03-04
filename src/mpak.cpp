//****************************************************************************
// "Those Funny Funguloids!"
// http://funguloids.sourceforge.net
// Copyright (c) 2006-2007, Mika Halttunen
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you must not
//  claim that you wrote the original software. If you use this software in a
//  product, an acknowledgment in the product documentation would be
//  appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and must not
//  be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source distribution.
//
//***************************************************************************/

// MPak package routines v1.31
// Copyright (c) 2004, 2007, Mika Halttunen
//
// MPak is a very simple package system of mine, which is used both to create
// MPAK package files and to read things from them. Very basic stuff, but hey,
// it works for me :)
// Note that MPak assumes that sizeof(UINT32) == 4

// Version history:
// v1.0:
//   - The first version.
// v1.1:
//  - MPak code should now be endian independent.
//  - Data is saved/read in little endian, and swapped when necessary
//  - As I don't have a big endian system to test this on, feel free to
//    report me if there's something wrong. To contact me: lsoft@mbnet.fi
// v1.2:
//  - Added CRC32 checksum which makes sure that the data isn't corrupted
// v1.3:
//  - The file sizes are no longer saved into the MPK file, but computed from
//    the offsets at load time. This saves 4 bytes per each stored file :)
// v1.31:
//  - Minor changes for Ogre integration

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpak.h"


//////////////////////////////////////////////
// CRC32 checksum stuff
//////////////////////////////////////////////

// This is based on Mark Nelsons article "File Verification Using CRC"
// from Dr. Dobb's Journal, May 1992
// http://www.dogma.net/markn/articles/crcman/crcman.htm


// CRC32 table
static UINT32 mpk_crc_table[256];

// Is the CRC table already built?
static bool mpk_crc_built = false;


// Build the CRC table
void mpk_build_crc_table() {
	UINT32 crc;
	for(int f=0; f < 256; f++) {
		crc = f;
		for(int i=8; i > 0; i--) {
			if(crc & 1)
				crc = (crc >> 1) ^ 0xEDB88320L;
			else
				crc >>= 1;
		}

		mpk_crc_table[f] = crc;
	}
}


// Compute the running CRC for a buffer of bytes
UINT32 mpk_compute_crc(UINT32 crc, size_t len, void *buffer) {
	unsigned char *p = (unsigned char*)buffer;
	while(len-- != 0) {
		UINT32 tmp1 = (crc >> 8) & 0x00FFFFFFL;
		UINT32 tmp2 = mpk_crc_table[((int)crc ^ *p++) & 0xFF];
		crc = tmp1 ^ tmp2;
	}

	return crc;
}


// Compute the CRC for a file
UINT32 mpk_crc_file(const char *file, UINT32 pos) {
    // Initial CRC
	UINT32 crc = 0xFFFFFFFFL;

	// Open the file
	FILE *fin = fopen(file, "rb");
	if(!fin)
		return 0;

	// Seek to the right position
	fseek(fin, pos, SEEK_SET);

	// Read the data in 16kb chunks and update the CRC sum
	while(1) {
		char buffer[16384];
		size_t len = fread(buffer, 1, sizeof(buffer), fin);
		if(len == 0)
			break;

		crc = mpk_compute_crc(crc, len, buffer);
	}

	crc ^= 0xFFFFFFFFL;

	// Close the file
	fclose(fin);
	return crc;
}


//////////////////////////////////////////////
// Endianess conversion stuff
//////////////////////////////////////////////

#define MPAK_BIG_ENDIAN			0
#define MPAK_LITTLE_ENDIAN		1

// Holds the system endianess, detected at MPAK_FILE::init()
static int mpk_endianess;


// Swaps UINT32 endianess
UINT32 mpk_swap_uint32(UINT32 num) {
	return ((num<<24)|((num<<8)&0x00FF0000)|((num>>8)&0x0000FF00)|(num>>24));
}


// Detects the system endianess.
void mpk_detect_endianess() {
	short int num = 0x01;
	mpk_endianess = *((char*) &num);
}


// This swaps UINT32 to/from little endian format, if necessary
UINT32 MPAK_SWAPLE(UINT32 x) {
	if(mpk_endianess == MPAK_BIG_ENDIAN)
		return mpk_swap_uint32(x);
	else
		return x;
}


//////////////////////////////////////////////
// The MPAK stuff
//////////////////////////////////////////////


// This helper function returns the filename without full path.
// Pass a full path.
char *mpk_strip_path(const char *path) {

	size_t pos = strlen(path);
	while(pos > 0) {
		char c = path[pos-1];
		if(c == '/' || c == '\\')
			break;

		pos--;
	}

	return (char*)path + pos;
}


// Initializes the MPAK_FILE structure.
// You must call init() once before using any of the other functions.
void MPAK_FILE::init() {
	num_files = 0;
	crc_checksum = 0;
	mode = MPAK_CLOSED;
	fpointer = NULL;
	filetable_offset = 0;
	strcpy(mpk_file, "null");
	strcpy(override_dir, "null");
	for(int f=0; f<MPAK_MAX_FILES; f++) {
		strcpy(files[f], "");
		offsets[f] = 0;
		sizes[f] = 0;
	}
	current_file_size = 0;
	current_file_offset = 0;
	current_file_overridden = false;

	// Detect the endianess of the system
	mpk_detect_endianess();

	// Build the CRC table (if not already built)
	if(!mpk_crc_built) {
		mpk_build_crc_table();
		mpk_crc_built = true;
	}
}


// Open a package for reading or writing. You don't need to supply
// the override directory, but it's recommended for reading mode.
// Returns zero on failure.
int MPAK_FILE::open_mpk(int open_mode, const char *file, const char *override) {

	// Check if it's already open
	if(mode != MPAK_CLOSED)
		return 0;

	// Check the mode, it must be either MPAK_READ or MPAK_WRITE
	mode = open_mode;
	if(mode != MPAK_READ && mode != MPAK_WRITE) {
		mode = MPAK_CLOSED;
		return 0;
	}

	// Try to open the file
	fpointer = fopen(file, (mode == MPAK_READ) ? "rb" : "wb");
	if(!fpointer) {
		mode = MPAK_CLOSED;
		return 0;
	}

	// Grab the file name
	strcpy(mpk_file, file);
	strcpy(override_dir, "null");

	// Are we reading or writing?
	if(mode == MPAK_WRITE) {
		// Write the ID
		fputs("MPK1", fpointer);

		// Reserve space for the CRC32 checksum
		UINT32 dummy_crc = 0;
		dummy_crc = MPAK_SWAPLE(dummy_crc);
		fwrite(&dummy_crc, sizeof(dummy_crc), 1, fpointer);

		// Reserve space for the file table offset.
		UINT32 dummy_offset = 0;
		dummy_offset = MPAK_SWAPLE(dummy_offset);
		fwrite(&dummy_offset, sizeof(dummy_offset), 1, fpointer);

		// Ok, we're ready for adding files.
		return 1;
	}
	else {
		// Read the ID
		char id[5] = "";
		id[0] = fgetc(fpointer);
		id[1] = fgetc(fpointer);
		id[2] = fgetc(fpointer);
		id[3] = fgetc(fpointer);
		id[4] = '\0';
		if(strcmp("MPK1", id) != 0) {
			mode = MPAK_CLOSED;
			return 0;					// Not a valid MPK1 package
		}

		// Read the CRC32 checksum
		fread(&crc_checksum, sizeof(crc_checksum), 1, fpointer);
		crc_checksum = MPAK_SWAPLE(crc_checksum);

		// Calculate the CRC32 of the file
		UINT32 crc = mpk_crc_file(mpk_file, 8);

		// Compare the checksum
		if(crc != crc_checksum) {
			mode = MPAK_CLOSED;
			return 0;					// Corrupted package
		}


		// Read the file table offset
		fread(&filetable_offset, sizeof(filetable_offset), 1, fpointer);
		filetable_offset = MPAK_SWAPLE(filetable_offset);

		// Seek the file table and read it
		fseek(fpointer, filetable_offset, SEEK_SET);

		// Read the number of files
		fread(&num_files, sizeof(num_files), 1, fpointer);
		num_files = MPAK_SWAPLE(num_files);


		// Read the file information
		UINT32 f=0;
		for(f=0; f<num_files; f++) {
			// File name length
			int len = fgetc(fpointer);
			// Filename
			fread(files[f], 1, len, fpointer);
			// Offset
			fread(&offsets[f], sizeof(offsets[f]), 1, fpointer);
			offsets[f] = MPAK_SWAPLE(offsets[f]);
			// Size (in bytes)
//			fread(&sizes[f], sizeof(sizes[f]), 1, fpointer);
//			sizes[f] = MPAK_SWAPLE(sizes[f]);
		}

		// Compute the sizes from the offsets
		for(f=0; f<num_files-1; f++)
			sizes[f] = offsets[f+1] - offsets[f];
		sizes[num_files-1] = filetable_offset - offsets[num_files-1];

		// We don't actually need to keep the file open anymore,
		// because open_file() opens it's own pointer to the file,
		// so close the package.
		fclose(fpointer);

		// Grab the override directory
		if(override != NULL)
			strcpy(override_dir, override);
		return 1;
	}
}


// Close the package. This writes the file table to the end of the
// package and updates the header. After closing the file, you can
// no longer add any files to the package.
void MPAK_FILE::close_mpk() {

	// Check that we're open
	if(mode != MPAK_WRITE && mode != MPAK_READ)
		return;

	// If we were reading, just "close" the package
	if(mode == MPAK_READ) {
		// The fpointer itself has been closed after reading the file table,
		// so we don't do it here.
		mode = MPAK_CLOSED;
		return;
	}
	else {
		// We're in write mode, so write the file table to the end of the file.
		// First we get the correct filetable offset.
		filetable_offset = ftell(fpointer);

		// Write...

		// ..the number of files
		num_files = MPAK_SWAPLE(num_files);
		fwrite(&num_files, sizeof(num_files), 1, fpointer);
		num_files = MPAK_SWAPLE(num_files);

		// ..the file information
		for(UINT32 f=0; f<num_files; f++) {
			// File name length
			size_t len = strlen(files[f])+1;
			fputc((int)len, fpointer);
			// Filename
			fwrite(files[f], 1, len, fpointer);
			// Offset
			offsets[f] = MPAK_SWAPLE(offsets[f]);
			fwrite(&offsets[f], sizeof(offsets[f]), 1, fpointer);
			offsets[f] = MPAK_SWAPLE(offsets[f]);
			// Size (in bytes)
//			sizes[f] = MPAK_SWAPLE(sizes[f]);
//			fwrite(&sizes[f], sizeof(sizes[f]), 1, fpointer);
//			sizes[f] = MPAK_SWAPLE(sizes[f]);
		}

		// Now we update the header to have the correct file table offset.
		fseek(fpointer, 8, SEEK_SET);
		filetable_offset = MPAK_SWAPLE(filetable_offset);
		fwrite(&filetable_offset, sizeof(filetable_offset), 1, fpointer);
		filetable_offset = MPAK_SWAPLE(filetable_offset);

		// Calculate the CRC checksum
		fflush(fpointer);
		fseek(fpointer, 4, SEEK_SET);
		crc_checksum = mpk_crc_file(mpk_file, 8);

		// ..and store it to the file
 		crc_checksum = MPAK_SWAPLE(crc_checksum);
		fwrite(&crc_checksum, sizeof(crc_checksum), 1, fpointer);
		crc_checksum = MPAK_SWAPLE(crc_checksum);

		// ..and now we're done.
		fclose(fpointer);
		mode = MPAK_CLOSED;
	}
}


// Add a file to the package.
// Returns zero on failure.
int MPAK_FILE::add_file(const char *file) {

	// Check that we're in write mode
	if(mode != MPAK_WRITE)
		return 0;

	// Try to open the file in question
	FILE *fin = fopen(file, "rb");
	if(!fin)
		return 0;		// Unable to open the file

	// Check the file size
	fseek(fin, 0, SEEK_END);
	UINT32 bytes = ftell(fin);
	if(bytes == 0)
		return 0;		// An empty file

	int cur = num_files;

	// Save the information to the file table
	strcpy(files[cur], mpk_strip_path(file));
	sizes[cur] = bytes;
	offsets[cur] = ftell(fpointer);

	// Transfer the source file in 16kb chunks to the package
	fseek(fin, 0, SEEK_SET);
	char buffer[16384];
	while(1) {
		size_t len = fread(buffer, 1, sizeof(buffer), fin);
		if(len == 0)
			break;

		fwrite(buffer, 1, (unsigned)len, fpointer);
	}
	fclose(fin);

	num_files++;
	if(num_files > MPAK_MAX_FILES)
		return 0;

	return 1;
}


// Get a pointer to a certain file in the package. It first looks
// from the override directory, and if the file isn't there it looks
// from the package. The user MUST fclose() the pointer himself!
// Returns NULL on failure.
FILE *MPAK_FILE::open_file(const char *file) {

	// Check that we're in read mode
	if(mode != MPAK_READ)
		return NULL;

	// First, look from the override directory.
	if(strcmp(override_dir, "null") != 0) {
		// Ok, the user has specified an override directory.
		char testfile[256] = "";
		sprintf(testfile, "%s/%s", override_dir, file);
		FILE *test = fopen(testfile, "rb");
		if(test) {
			// Yup, there was a file. Give this pointer to the user.
			// It's up to the user to fclose() it.

			// Save the size of the file.
			fseek(test, 0, SEEK_END);
			current_file_size = ftell(test);
			current_file_offset = 0;
			current_file_overridden = true;
			fseek(test, 0, SEEK_SET);
			return test;
		}
	}

	// Look from the file table
	int idx = find_file(file);
	if(idx != -1) {
		// We've found the file. Get a pointer to it and
		// return it to the user.
		FILE *pointer = fopen(mpk_file, "rb");
		if(!pointer)
			return NULL;

		// Seek to the correct offset and return the pointer.
		// It's up to the user to fclose() it.
		fseek(pointer, offsets[idx], SEEK_SET);
		current_file_size = sizes[idx];
		current_file_offset = offsets[idx];
		current_file_overridden = false;
		return pointer;
	}

	// The file was not found :/
	return NULL;
}


// Extracts a file from the package (using open_file()) and saves it
// to a file by the same name. It first looks from the override directory,
// and if the file isn't there it looks from the package.
// You can supply optional path prefix if you don't want to extract to the
// working directory.
// Returns zero on failure.
int MPAK_FILE::extract_file(const char *file, const char *path) {

	// Retrieve the pointer from open_file()
	FILE *fin = open_file(file);
	if(!fin)
		return 0;				// There wasn't such a file!

	// Determine the file name
	char file_out[256] = "";
	if(path != NULL)
		sprintf(file_out, "%s/%s", path, file);
	else
		strcpy(file_out, file);

	// Open a stream for writing the file into
	FILE *fout = fopen(file_out, "wb");
	if(!fout) {
		fclose(fin);
		return 0;				// Unable to open the out file!
	}

	// Read in 16kb chunks and spit them out
	char buffer[16384];
	UINT32 bytes_written = 0;
	UINT32 total_bytes = current_file_size;

	while(1) {
		// We have to watch not to write too much.
		UINT32 bytes_left = total_bytes - bytes_written;
		if(bytes_left > 16384)
			bytes_left = 16384;

		fread(buffer, 1, bytes_left, fin);
		fwrite(buffer, 1, bytes_left, fout);
		bytes_written += bytes_left;

		if(bytes_written >= total_bytes)
			break;
	}

	// Close the pointers.
	fclose(fin);
	fclose(fout);

	return 1;
}


// Find the index of a particular file in the package.
// Returns -1 on failure.
int MPAK_FILE::find_file(const char *file) {
	// Check that we're open
	if(mode == MPAK_CLOSED)
		return -1;

	for(UINT32 f=0; f<num_files; f++)
		if(strcmp(files[f], file) == 0)
			return f;


	return -1;
}


