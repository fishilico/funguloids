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

// Global high score list
HiscoreList hiscoreList;


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
void HiscoreList::save(char *file) {
	FILE *fout;
	fout = fopen(file, "wb");
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
void HiscoreList::load(char *file) {
	FILE *fin;
	fin = fopen(file, "rb");
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
int HiscoreList::addName(char *name, long score) {
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
