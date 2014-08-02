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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "game.h"
#include "hiscore.h"
#include <OgreTextAreaOverlayElement.h>
#include <OgreOverlayManager.h>

// Global high score list
HiscoreList hiscoreList;

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifndef FUNGULOIDS_HIGHSCORE_PATH // this path needs rw access
  #define FUNGULOIDS_HIGHSCORE_PATH getFunguloidsDir()
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
// The hiscore files are saved to the users HOME directory in Linux, but
// not in Windows.
#include <string>
#include <sys/stat.h>
#include <dirent.h>


// Helper function which gets the .funguloids directory from the
// user's home directory. It creates the dir if it doesn't exist.
String getFunguloidsDir() {
	// Get the HOME environment variable
	char *home = getenv("HOME");
	if(home != NULL) {
		String tmp = String(home) + "/.funguloids/";
		// Check if the directory exists
		DIR *dp = NULL;
		dp = opendir(tmp.c_str());
		if(!dp) {
			// It doesn't exist, try to create it
			if(mkdir(tmp.c_str(), S_IRWXU))
				// Can't create it
				return "";
			return tmp;
		}
		else {
			// It does exist.
			closedir(dp);
			return tmp;
		}
	}

	// The HOME variable wasn't available
	return "";
}
#endif

// Helper function which returns suitable path for the
// hiscore file. It first checks the user's home directory,
// (in Linux only) and if that fails it uses the default directory.
String getHiscoreLocation(bool reading) {
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	// Get the path to the hiscore file
//	String tmp = getFunguloidsDir() + HISCORE_FILE;
	String tmp = String(FUNGULOIDS_HIGHSCORE_PATH) + HISCORE_FILE;

	// Check if the hiscore file exists there
	if(reading) {
		FILE *ftest = fopen(tmp.c_str(), "rt");
		if(!ftest) {
			// It doesn't exist, try the default
			return HISCORE_FILE;
		}
		fclose(ftest);
	}

	return tmp;
#endif

	// Return the default
	return HISCORE_FILE;
}



// Update the overlay
void HiscoreList::updateOverlay() {
	for(int f=0; f<NUM_NAMES; f++) {
		Real a = 1.0f - ((Real)f/(Real)NUM_NAMES) * 0.3f;
		TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Name" + StringConverter::toString(f+1)));
		item->setCaption(String(mList[f].name));
		item->setColourTop(ColourValue(0.8f , 0.4f , 0.3f ));
		item->setColourBottom(ColourValue(1*a, 0.8f*a, 0.7f*a));

		item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Score" + StringConverter::toString(f+1)));
		char str[16] = "";
		sprintf(str, "%06ld", mList[f].score);
		item->setCaption(String(str));
		item->setColourTop(ColourValue(0.8f, 0.6f, 0.4f));
		item->setColourBottom(ColourValue(a,a,a));
	}
}


// Save the list to a file
void HiscoreList::save(const String &file) {
	FILE *fout;
	fout = fopen(file.c_str(), "wb");
	if(!fout) {
		LogManager::getSingleton().logMessage("Unable to save the hiscore list!");
		return;
	}

	// Write the scores
	int f;
	for(f=0; f<NUM_NAMES; f++)
		fwrite(&mList[f].score, sizeof(long), 1, fout);

	// Write the names
	for(f=0; f<NUM_NAMES; f++)
		fwrite(mList[f].name, NAME_LEN, 1, fout);

	fclose(fout);
}


// Load the list from a file
void HiscoreList::load(const String &file) {
	FILE *fin;
	fin = fopen(file.c_str(), "rb");
	if(!fin) {
		// The list wasn't found, no problem.
		clear();
		return;
	}

	// Read the scores
	int f;
	for(f=0; f<NUM_NAMES; f++)
		fread(&mList[f].score, sizeof(long), 1, fin);

	// Read the names
	for(f=0; f<NUM_NAMES; f++)
		fread(mList[f].name, NAME_LEN, 1, fin);

	fclose(fin);
}


// Add a name to the list. Returns the place.
int HiscoreList::addName(const char *name, long score) {
	// Check if we qualify
	if(mList[NUM_NAMES-1].score >= score)
		return -1;

	// Replace the last entry
	memset(mList[NUM_NAMES-1].name, 0, sizeof(mList[NUM_NAMES-1].name));
	strncpy(mList[NUM_NAMES-1].name, name, NAME_LEN);
	mList[NUM_NAMES-1].score = score;

	// Sort
	sort();

	// Find ourselves and return the place
	for(int f=NUM_NAMES-1; f >= 0; f--)
		if(strcmp(name, mList[f].name) == 0 && score == mList[f].score)
			return f;

	// This shouldn't happen..
	return -1;
}


// Comparison function for qsort
int sort_cmp(const void *a, const void *b) {
	Record ra = *(Record*)a;
	Record rb = *(Record*)b;

	// Sort by the score
	if(ra.score < rb.score)
		return 1;
	else if(ra.score > rb.score)
		return -1;

	// The scores are same, sort by the name
	return strcmp(ra.name, rb.name);
}


// Sort the list using qsort
void HiscoreList::sort() {
	qsort(mList, NUM_NAMES, sizeof(Record), sort_cmp);
}


// Clear the list
void HiscoreList::clear() {
	for(int f=0; f<NUM_NAMES; f++) {
		memset(mList[f].name, 0, sizeof(mList[f].name));
		strncpy(mList[f].name, "- - - - - - - - - - - - - - - -", NAME_LEN);
		mList[f].score = 0;
	}
}
