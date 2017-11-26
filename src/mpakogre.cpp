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

#include <OgreStableHeaders.h>
#include <OgreLogManager.h>
#include <OgreException.h>
#include <OgreStringVector.h>
#include <OgreRoot.h>

#include "mpakogre.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include <OgreSearchOps.h>
#   include <sys/param.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   define MAX_PATH MAXPATHLEN
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#   include <windows.h>
#   include <direct.h>
#   include <io.h>
#endif


MPakArchive::MPakArchive(const String &name, const String &archType) : Archive(name, archType) {
	mPakFile = NULL;
}

MPakArchive::~MPakArchive() {
	unload();
}


void MPakArchive::load() {
	if(!mPakFile) {
		mPakFile = new MPAK_FILE();
		mPakFile->init();
		if(!mPakFile->open_mpk(MPAK_READ, mName.c_str(), ((mName == "bootstrap.mpk") ? NULL : "media"))) {
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Unable to open " + mName + "! It either doesn't exist or is corrupted", "MPakArchive::load");
			return;
		}

		// Fill the file infos
		for(unsigned int f=0; f<mPakFile->num_files; f++) {
			FileInfo info;
			info.archive = this;
			info.basename = String(mPakFile->files[f]);
			info.filename = String(mPakFile->files[f]);
			info.compressedSize = static_cast<size_t>(mPakFile->sizes[f]);
			info.uncompressedSize = info.compressedSize;

			mFileList.push_back(info);
		}

		// The following is commented out since it isn't really needed,
		// I'm not planning to use the override directory.. It wasn't compiling on
		// Linux anyways, as the _find* aren't exported currently (Ogre Eihort 1.4.0)
#if 0
		// Find the possible files in the override directory (media) and add them too
		if(mPakFile->override_dir != NULL) {
			// Start searching
			long handle, res = 0;
			struct _finddata_t findData;
			handle = _findfirst((String(mPakFile->override_dir) + "/*.*").c_str(), &findData);
			while(handle != -1 && res != -1) {
				// Ignore directories
				if(!(findData.attrib & _A_SUBDIR)) {
					// Add the file if it's not already in the resources / MPK file
					bool res1 = ResourceGroupManager::getSingleton().resourceExists(ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, findData.name);
					bool res2 = ResourceGroupManager::getSingleton().resourceExists(ResourceGroupManager::BOOTSTRAP_RESOURCE_GROUP_NAME, findData.name);
					
					if(mPakFile->find_file(findData.name) == -1 && !res1 && !res2) {
						String name = String(findData.name);

						// Add the file
						FileInfo info;
						info.archive = this;
						info.basename = name;
						info.path = String(mPakFile->override_dir) + '/';
						info.filename = name;
						info.uncompressedSize = findData.size;
						info.compressedSize = findData.size;
						mFileList.push_back(info);
					}
				}

				res = _findnext(handle, &findData);
			}
			
			if(handle != -1)
				_findclose(handle);
		}
#endif

	}
}

void MPakArchive::unload() {
	if(mPakFile) {
		mPakFile->close_mpk();
		mFileList.clear();
		delete mPakFile;
		mPakFile = NULL;
	}
}


DataStreamPtr MPakArchive::open(const String &filename) const {
	FILE *fil = mPakFile->open_file(filename.c_str());
    if(!fil) {
		LogManager::getSingleton().logMessage(mName + " - Unable to open file " + filename + "!");
		return DataStreamPtr();
	}


	// Construct and return stream
	DataStreamPtr mpak(new MPakDataStream(filename, fil, (size_t)mPakFile->current_file_size, (size_t)mPakFile->current_file_offset));
	return DataStreamPtr(new MemoryDataStream(filename, mpak));
    //return DataStreamPtr(new MPakDataStream(filename, fil, (size_t)mPakFile->current_file_size, (size_t)mPakFile->current_file_offset));
}


StringVectorPtr MPakArchive::list(bool recursive, bool dirs) const {
	StringVectorPtr ret = StringVectorPtr(new StringVector());

	FileInfoList::const_iterator i, iend;
	iend = mFileList.end();
	for(i = mFileList.begin(); i != iend; ++i) {
		ret->push_back(i->filename);
	}

	return ret;
}

FileInfoListPtr MPakArchive::listFileInfo(bool recursive, bool dirs) const {
	FileInfoList *fil = new FileInfoList();
	FileInfoList::const_iterator i, iend;
	iend = mFileList.end();
	for(i = mFileList.begin(); i != iend; ++i) {
		fil->push_back(*i);
	}

	return FileInfoListPtr(fil);
}

StringVectorPtr MPakArchive::find(const String &pattern, bool recursive, bool dirs) const {
	StringVectorPtr ret = StringVectorPtr(new StringVector());

	FileInfoList::const_iterator i, iend;
	iend = mFileList.end();
	for(i = mFileList.begin(); i != iend; ++i) {
		if(StringUtil::match(i->filename, pattern, true))
			ret->push_back(i->filename);
	}

	return ret;
}

FileInfoListPtr MPakArchive::findFileInfo(const String &pattern, bool recursive, bool dirs) const {
	FileInfoListPtr ret = FileInfoListPtr(new FileInfoList());

	FileInfoList::const_iterator i, iend;
	iend = mFileList.end();
	for(i = mFileList.begin(); i != iend; ++i) {
		if(StringUtil::match(i->filename, pattern, true))
			ret->push_back(*i);
	}

	return ret;
}

bool MPakArchive::exists(const String &filename) const {
	int i = mPakFile->find_file(filename.c_str());
	return (i != -1);
}

const String &MPakArchiveFactory::getType(void) const {
	static String name = "MPK";
	return name;
}


MPakDataStream::MPakDataStream(FILE *handle, size_t size, size_t offset) : DataStream(), mFileHandle(handle) {
	mSize = size;
	mOffset = offset;
}

MPakDataStream::MPakDataStream(const String &name, FILE *handle, size_t size, size_t offset) : DataStream(name), mFileHandle(handle) {
	mSize = size;
	mOffset = offset;
}

MPakDataStream::~MPakDataStream() {
	close();
}

size_t MPakDataStream::read(void *buf, size_t count) {
	return fread(buf, 1, count, mFileHandle);
}

void MPakDataStream::skip(long count) {
	fseek(mFileHandle, count, SEEK_CUR);
}

void MPakDataStream::seek(size_t pos) {
	fseek(mFileHandle, (long)(pos + mOffset), SEEK_SET);
}

size_t MPakDataStream::tell(void) const {
	return ftell(mFileHandle) - mOffset;
}

bool MPakDataStream::eof(void) const {
	//return tell() == mSize;
	return feof(mFileHandle) != 0;
}

void MPakDataStream::close(void) {
	fclose(mFileHandle);
	mFileHandle = 0;
}
