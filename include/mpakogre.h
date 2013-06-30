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

#ifndef MPAKOGRE_H
#define MPAKOGRE_H

#include <OgrePrerequisites.h>

#include <OgreArchive.h>
#include <OgreArchiveFactory.h>
#include "mpak.h"

using namespace Ogre;

// Archive class for reading from MPK files
class MPakArchive : public Archive {
protected:
	MPAK_FILE *mPakFile;
	FileInfoList mFileList;

public:
	MPakArchive(const String &name, const String &archType);
	~MPakArchive();

	bool isCaseSensitive(void) const { return true; }
	void load();
	void unload();

	DataStreamPtr open(const String &filename) const;
	DataStreamPtr open(const String &filename, bool) const { return open(filename); };
	StringVectorPtr list(bool recursive = true, bool dirs = false);
	FileInfoListPtr listFileInfo(bool recursive = true, bool dirs = false);

	StringVectorPtr find(const String &pattern, bool recursive = true, bool dirs = false);
	FileInfoListPtr findFileInfo(const String &pattern, bool recursive, bool dirs = false) const;

	bool exists(const String &filename);

	time_t getModifiedTime(const String& filename)
	{
		struct stat tagStat;
		bool ret = (stat(mName.c_str(), &tagStat) == 0);

		if (ret)
		{
			return tagStat.st_mtime;
		}
		else
		{
			return 0;
		}

	}
};


// Archive factory for MPK files
class MPakArchiveFactory : public ArchiveFactory {
public:
	MPakArchiveFactory() : ArchiveFactory() {}
	virtual ~MPakArchiveFactory() {}
	const String &getType(void) const;

	Archive *createInstance(const String &name) {
		return new MPakArchive(name, "MPK");
	}

	void destroyInstance(Archive *arch) { delete arch; }
};


// Data stream for MPK files
class MPakDataStream : public DataStream {
protected:
	FILE *mFileHandle;
	size_t mOffset;

public:
	MPakDataStream(FILE *handle, size_t size, size_t offset = 0);
	MPakDataStream(const String &name, FILE *handle, size_t size, size_t offset = 0);
	~MPakDataStream();

	size_t read(void *buf, size_t count);
	void skip(long count);
	void seek(size_t pos);
	size_t tell(void) const;
	bool eof(void) const;
	void close(void);
};

#endif
