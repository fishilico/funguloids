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


#include "ogreapp.h"
#include "playlist.h"
#include <vector>
#include <algorithm>

#include <sys/types.h>
#include <sys/stat.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX || OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#   include "OgreSearchOps.h"
#   include <sys/param.h>
#   define MAX_PATH MAXPATHLEN
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#   include <windows.h>
#   include <direct.h>
#   include <io.h>
#endif


using namespace std;

// Play list files
static vector<String> playList;
static unsigned int currentSong = -1;


// Initialise the playlist
void initPlaylist() {
	playList.clear();
	currentSong = -1;
}


// Add files to the playlist (wildcards supported)
void addToPlaylist(const String &files) {
	int count = 0;
	String path = files.substr(0, files.find_last_of('/')) + '/';
	if(path == (files + '/')) path = "";

	// Start searching
	long handle, res = 0;
	struct _finddata_t findData;
	handle = _findfirst(files.c_str(), &findData);
	while(handle != -1 && res != -1) {
		// Ignore directories
		if(!(findData.attrib & _A_SUBDIR)) {
			String name = String(findData.name);

			// Add the file to the playlist
			if(name != "playlist.lua") {
				String fullPath = path + name;
				playList.push_back(fullPath);
				count++;
			}
		}

		res = _findnext(handle, &findData);
	}
	
	if(handle != -1)
		_findclose(handle);

	LogManager::getSingleton().logMessage("Added " + StringConverter::toString(count) + " files to the playlist.");
}


// Shuffle the playlist
void shufflePlaylist() {
	if(playList.empty()) return;
	random_shuffle(playList.begin(), playList.end());
}


// Return the next song from the play list
String getNextSong() {
	if(playList.empty()) return "";
	currentSong++;
	if(currentSong >= playList.size()) currentSong = 0;
	return playList[currentSong];
}


// Mark the current song as bad (i.e. not playable)
void markBadSong() {
	if(playList.empty()) return;
	playList[currentSong] = "";
}
