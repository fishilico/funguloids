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

#ifndef MPAK_H
#define MPAK_H

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

// Maximum number of files in the package
#define MPAK_MAX_FILES			256
//#define MPAK_MAX_FILES			1024

// Modes
#define MPAK_READ				0
#define MPAK_WRITE    			1
#define MPAK_CLOSED				2


// Define UINT32
typedef unsigned int UINT32;



// Struct representing a MPak file
struct MPAK_FILE {
	UINT32 num_files;						// Number of files
	UINT32 crc_checksum;					// CRC32 checksum
	char override_dir[64];					// Name of the override directory
	FILE *fpointer;       					// Pointer to the MPK file
	char mpk_file[64];						// Filename of the open MPK file
	int mode;								// Opened for writing or reading?

	char files[MPAK_MAX_FILES][64];			// File names
	UINT32 filetable_offset;     			// Offset to the file table
	UINT32 offsets[MPAK_MAX_FILES];			// File offsets
	UINT32 sizes[MPAK_MAX_FILES];			// File sizes
	UINT32 current_file_size;				// Size of the current file being open
	UINT32 current_file_offset;				// Offset of the current file being open
	bool current_file_overridden;			// Is the current file overridden?


	// Functions

	// Initializes the MPAK_FILE structure.
	// You must call init() once before using any of the other functions.
	void init();

	// Open a package for reading or writing. You don't need to supply
	// the override directory, but it's recommended for reading mode.
	// Returns zero on failure.
	int open_mpk(int mode, const char *file, const char *override = NULL);

	// Close the package. This writes the file table to the end of the
	// package and updates the header. After closing the file, you can
	// no longer add any files to the package.
	void close_mpk();

	// Add a file to the package.
	// Returns zero on failure.
	int add_file(const char *file);

	// Get a pointer to a certain file in the package. It first looks
	// from the override directory, and if the file isn't there it looks
	// from the package. The user MUST fclose() the pointer himself!
	// Returns NULL on failure.
	FILE *open_file(const char *file);

	// Extracts a file from the package (using open_file()) and saves it
	// to a file by the same name. It first looks from the override directory,
	// and if the file isn't there it looks from the package.
	// You can supply optional path prefix if you don't want to extract to the
	// working directory.
	// Returns zero on failure.
	int extract_file(const char *file, const char *path = NULL);

	// Find the index of a particular file in the package.
	// Returns -1 on failure.
	int find_file(const char *file);

};

#endif

