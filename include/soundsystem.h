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

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <Ogre.h>
#include <OgreSingleton.h>

#ifdef HAVE_CONFIG_H
	#include "config.h"
#else
	// TODO: fallback/error if autoconf fails or whatever
#endif

#ifdef SOUND_SYSTEM_OPENAL
	#include "openalsoundsystem.h"
#else
	#ifdef SOUND_SYSTEM_FMOD
		#include "fmodexsoundsystem.h"
	#endif
#endif

using namespace Ogre;
using namespace std;

// Sound resource
class SoundResource {
public:
	Archive *fileArchive;
	DataStreamPtr streamPtr;
	String fileName;
};

// File locator for sound files
// (based on wiki example at http://www.ogre3d.org/wiki/index.php/File_SoundManager.cpp)
class SoundLocator : public ResourceGroupManager {
public:
	SoundLocator() {}
	~SoundLocator() {}

	Archive *findSound(String &filename) {
		ResourceGroup *grp = getResourceGroup("General");
		if(!grp)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate a resource group called 'General'", "ResourceGroupManager::openResource");

		OGRE_LOCK_MUTEX(grp->OGRE_AUTO_MUTEX_NAME) // lock group mutex
		ResourceLocationIndex::iterator rit = grp->resourceIndexCaseSensitive.find(filename);
		if(rit != grp->resourceIndexCaseSensitive.end()) {
			// Found in the index
			Archive *fileArchive = rit->second;
			filename = fileArchive->getName() + "/" + filename;
			return fileArchive;
		}

		return NULL;
	}
};

#endif
