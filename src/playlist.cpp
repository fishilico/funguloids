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

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#else
#    include <stdlib.h>
#    include <stdio.h>
#    include <string.h>
#    include <dirent.h>
#endif


using namespace std;

// Play list files
static std::vector<String> playList;
static unsigned int currentSong = -1;


// Initialise the playlist
void initPlaylist() {
	playList.clear();
	currentSong = -1;
}


// Add files to the playlist (wildcards supported)
void addToPlaylist(const String &files) {
	int count = 0;

	String path = files.substr(0, files.find_last_of('/'));
	String search = files.substr(files.find_last_of('/')+1);

// For Windows
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	WIN32_FIND_DATA findData;

	HANDLE handle = FindFirstFile(files.c_str(), &findData);
	if(handle != INVALID_HANDLE_VALUE) {
		String name = String(findData.cFileName);
		// Add the file to the playlist
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if(name != "playlist.lua") {
				String fullPath = path + "/" + name;
				playList.push_back(fullPath);
				count++;
			}
		}

		while(FindNextFile(handle, &findData)) {
			String name = String(findData.cFileName);
			if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				if(name != "playlist.lua") {
					String fullPath = path + "/" + name;
					playList.push_back(fullPath);
					count++;
				}
			}
		}

		FindClose(handle);
	}
// For Linux/Unix/etc.
#else
	DIR *dp;
	dirent *ep;

	dp = opendir(path.c_str());
	if(!dp)
		return;

	// Start searching
	while((ep = readdir(dp)) ) {
		String name = String(ep->d_name);

		// Skip directories
		if(ep->d_type == DT_DIR) continue;	// If this doesn't work, comment it and uncomment the following hack
		//if(ep->d_name[0] == '.') continue;
		// Cheap hack: if the filename doesn't have a dot in it, it's a directory.. :P
		//if(name.find_first_of('.') == String::npos) continue;

		if(name == "playlist.lua") continue;

		// Check if it matches the search
		if(StringUtil::match(name, search)) {
			String fullPath = path + "/" + name;
			playList.push_back(fullPath);
			count++;
		}
	}

	closedir(dp);
#endif

/*	String path = files.substr(0, files.find_last_of('/')) + '/';
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
*/

	LogManager::getSingleton().logMessage("Added " + StringConverter::toString(count) + " files from \"" + path + "\" to the playlist.");
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
