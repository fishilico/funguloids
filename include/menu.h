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

#ifndef MENU_H
#define MENU_H

#include "ogreapp.h"
#include <OgreTextAreaOverlayElement.h>

class HiscoreList;

class Menu {
public:
	Menu(SceneManager *mgr) {
		mSceneMgr = mgr;
		mSelection = 0;
		mMenuName = "MainMenu";
		mNumItems = findMenuItems();
		mNewState = -1;
		mItemAlpha = 0;
		mPrevSelection = 0;

		mEnteringName = false;
		mNewName = NULL;
		mNewScorePlace = -1;
		mNewItem = NULL;
	}

	int findMenuItems();
	void handleKeyPress(KeyEvent *e);
	void handleSelection();
	int getNewState() const { return mNewState; }
	void setMenu(const String &menu, int selection = 0);
	void show();
	void hide();
	void hilightItem(int i);
	void dimItem(int i);
	void fadeItem(Real delta);
	void updateOptionsItem(int i, bool change = false);
	void prepareOptionsItems();
	void scrollUp();
	void scrollDown();

	void enterName(int place);
	void doEnterName(KeyEvent *e);
	bool isEnteringName() const { return mEnteringName; }

private:
	int mSelection;					// Menu selection
	int mPrevSelection;				// Previous selectio
	int mNumItems;					// Number of menu items
	String mMenuName;				// Menu name
	int mNewState;					// New state to be set
	Real mItemAlpha;				// Menu item alpha fader
	SceneManager *mSceneMgr;		// Ogre scene manager

	bool mEnteringName;				// Are we entering name to the highscores?
	char *mNewName;					// Pointer to the new highscore name
	int mNewScorePlace;				// New highscore place
	TextAreaOverlayElement *mNewItem;	// Pointer to the overlay item
	int mTextPos;					// Text cursor position
};

#endif
