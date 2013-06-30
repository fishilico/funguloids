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

#ifndef HISCORE_H
#define HISCORE_H

// Number of names in the hiscore list
#define NUM_NAMES			10

// Name length
#define NAME_LEN			25

// Hiscore file
#define HISCORE_FILE		"hiscore.lst"


// Record structure
struct Record {
	char name[NAME_LEN];			// Name
	long score;						// Score
};


// Hiscore list
class HiscoreList {
public:
	Record mList[NUM_NAMES];					// Records

	void clear();								// Clear the list
	void sort();								// Sort the list
	int addName(const char *name, long score);	// Add a record
	void save(const String &file);					// Save the list
	void load(const String &file);					// Load the list
	void updateOverlay();						// Update the overlay
};

// Global high score list
extern HiscoreList hiscoreList;

// Helper function which returns suitable path for the
// hiscore file. It first checks the user's home directory,
// (in Linux only) and if that fails it uses the default directory.
String getHiscoreLocation(bool reading);

#endif
