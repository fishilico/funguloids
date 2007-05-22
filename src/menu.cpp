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

#include "menu.h"
#include "game.h"
#include "objectsystem.h"
#include "soundsystem.h"
#include "hiscore.h"
#include <OgreTextAreaOverlayElement.h>


// Version string
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
  #undef VERSION
  #define VERSION		"1.06"
#endif

// Menu items
enum {
	MENU_MAIN_START = 0,
	MENU_MAIN_OPTIONS,
	MENU_MAIN_HIGHSCORES,
	MENU_MAIN_EXIT,

	MENU_OPTIONS_BLOOM = 0,
	MENU_OPTIONS_SHADOWS,
	MENU_OPTIONS_DYN_LIGHTS,
	MENU_OPTIONS_DYN_LIGHT_SHADOWS,
	MENU_OPTIONS_SOUND_VOL,
	MENU_OPTIONS_MUSIC_VOL,
	MENU_OPTIONS_ACCEPT,
	MENU_OPTIONS_CANCEL,

	MENU_HISCORES_OK = 0,

	MENU_TITLE_OK
};


// Title screen alpha
static Real titleAlpha = 1.0f;


// Update the menu
void OgreAppFrameListener::doMenu(Real delta) {
	// Update all the moving objects (enemies, ...)
	ObjectMapType::iterator i;
	ObjectSystem *ob = ObjectSystem::getSingletonPtr();
	for(i=ob->getFirst(); i != ob->getLast(); ) {
		(*i).second->move(delta);

		// Destroy objects marked for deletion
		if((*i).second->isToBeDeleted())
			ob->destroyObject(i++);
		else
			++i;
	}


	// Fade the menu selection
	gameApp->getMenu()->fadeItem(delta);

	// Scroll the menu scroller
	Overlay *scroller = OverlayManager::getSingleton().getByName("MenuScroller");
	scroller->scroll(-0.15f * delta, 0);
	if(scroller->getScrollX() < -17) scroller->setScroll(0,0);

	// Handle the state switching
	int newState = gameApp->getMenu()->getNewState();
	if(newState == -1) return;

	if(newState == STATE_GAME) {
		// Black and White off
		Viewport *vp = gameApp->getRenderWindow()->getViewport(0);
		CompositorManager::getSingleton().setCompositorEnabled(vp, "B&W", false);

		// Scroller off
		OverlayManager::getSingleton().getByName("MenuScroller")->hide();
		OverlayManager::getSingleton().getByName("TitleScreen")->hide();
		OverlayManager::getSingleton().getByName("Version")->hide();

		// Start a new game
		gameApp->startNewGame();
		newState = -1;
	}
	else if(newState == STATE_QUIT) {
		gameApp->quit();
		newState = -1;
	}
}


// Fade the item selection
void Menu::fadeItem(Real delta) {
	// Fade the title screen here, too
	if(mMenuName != "TitleScreen") {
		if(titleAlpha > 0) {
			titleAlpha -= delta * 0.5f;
			if(titleAlpha < 0) {
				titleAlpha = 0;
				OverlayManager::getSingleton().getByName("TitleScreen")->hide();
			}

			// Update the alpha
			MaterialPtr mat = MaterialManager::getSingleton().getByName("TitleScreen");
			mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, titleAlpha);
		}
	}
	else {
		if(titleAlpha < 1.0f) {
			titleAlpha += delta * 0.75f;
			if(titleAlpha > 1.0f)
				titleAlpha = 1.0f;

			// Update the alpha
			MaterialPtr mat = MaterialManager::getSingleton().getByName("TitleScreen");
			mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, titleAlpha);
		}
	}


	if(mNumItems == 0) return;

	mItemAlpha += delta * 10;
	Real alpha = 0.75f + Math::Sin(Radian(mItemAlpha)) * 0.25f;

	TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mMenuName + StringConverter::toString(mSelection+1)));
	ColourValue col = item->getColourTop();
	col.a = alpha;
	item->setColourTop(col);

	col = item->getColourBottom();
	col.a = alpha;
	item->setColourBottom(col);
}


// Load a menu
void Menu::setMenu(const String &menu, int selection) {
	static bool firstTime = true;

	mMenuName = menu;
	mNumItems = findMenuItems();
	mSelection = selection;
	mNewState = -1;

	// Set the colors
	MaterialPtr mat = MaterialManager::getSingleton().getByName("BlockCenter");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 0.5f);
	mat = MaterialManager::getSingleton().getByName("BlockBorder");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 0.5f);

	for(int f=0; f<mNumItems; f++)
		dimItem(f);
	hilightItem(mSelection);

	// Black and White on
	Viewport *vp = gameApp->getRenderWindow()->getViewport(0);
	CompositorManager::getSingleton().setCompositorEnabled(vp, "B&W", true);

	// Scroller on
	OverlayManager::getSingleton().getByName("MenuScroller")->show();

	// Show the menu
	show();

	// Load the background
	if(firstTime && menu == "MainMenu") {
		gameApp->loadLevel("menubg.lua");

		// Load the high scores
		hiscoreList.load(getHiscoreLocation(true));
		firstTime = false;
		
		// Update the version string
		TextAreaOverlayElement *ver = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("VersionText"));
		ver->setCaption("v" VERSION);
	}

	// Update the options menu
	if(menu == "GameOptions")
		prepareOptionsItems();
	else if(menu == "HighScores")
		hiscoreList.updateOverlay();
	else if(menu == "TitleScreen")
		titleAlpha = 0.0f;
	else if(menu == "MainMenu")
		OverlayManager::getSingleton().getByName("Version")->show();
}


// Enter a new name to the high scores
void Menu::enterName(int place) {
	mEnteringName = true;
	mNewScorePlace = place;
	mNewName = hiscoreList.mList[place].name;
	strcpy(mNewName, "");
	mTextPos = 0;

	// Change the text
	TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("HighScores1"));
	item->setCaption("Congratulations!");

	// Change the colours
	mNewItem = item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Name" + StringConverter::toString(place+1)));
	item->setColourTop(ColourValue(0.7f, 0.7f, 0.7f));
	item->setColourBottom(ColourValue(1,1,1));
	mNewItem->setCaption("^");

	item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Score" + StringConverter::toString(place+1)));
	item->setColourTop(ColourValue(0.7f, 0.7f, 0.7f));
	item->setColourBottom(ColourValue(1,1,1));
}


// Do the name entering
void Menu::doEnterName(const OIS::KeyEvent &e) {
	bool typed = false;
	char ch = e.text;
	char *str = mNewName;

	// Check the input
	if(e.key == OIS::KC_BACK) {
		// Play a typing sound
		SoundSystem::getSingleton().playSound("typing");
		str[mTextPos] = '\0';
		mTextPos--;
		if(mTextPos < 0) mTextPos = 0;
		str[mTextPos] = '\0';
		// Update
		mNewItem->setCaption(String(str) + "^");
	}
	else if(e.key == OIS::KC_RETURN) {
		if(mTextPos > 0) {
			// Done, save
			hiscoreList.save(getHiscoreLocation(false));
			mEnteringName = false;

			TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("HighScores1"));
			item->setCaption("Back to main");

			// Update
			mNewItem->setCaption(String(str));
			hiscoreList.updateOverlay();
			return;
		}
	}
	else if(ch >= 32 && ch <= 126) {
		// Add a character
		typed = true;
		str[mTextPos] = ch;
	}

	// Modify string
	if(typed) {
		// Play a typing sound
		SoundSystem::getSingleton().playSound("typing");
		mTextPos++;
		if(mTextPos > NAME_LEN-1)
			mTextPos = NAME_LEN-1;
		str[mTextPos] = '\0';

		// Update
		mNewItem->setCaption(String(str) + "^");
	}
}


// Update an options menu item
void Menu::updateOptionsItem(int i, bool change) {
	TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mMenuName + StringConverter::toString(i+1)));
	Viewport *vp = gameApp->getRenderWindow()->getViewport(0);
	String itemText = item->getCaption();
	itemText = itemText.substr(0, itemText.find_last_of(':'));
	itemText += ": ";
	String str;
	char volstr[16] = "";
	Real vol = 0;
	
	switch(i) {
		// Bloom effect
		case MENU_OPTIONS_BLOOM:
			str = gameApp->mGameConfig->GetValue("graphics", "bloom", "on");
			if(change) {
				if(str == "on") str = "off";
				else if(str == "off") str = "on";
				gameApp->mGameConfig->SetValue("graphics", "bloom", str.c_str());
				CompositorManager::getSingleton().setCompositorEnabled(vp, "Bloom", (str != "off"));
			}
			item->setCaption(itemText + ((str == "on") ? "ON" : "OFF"));
			break;

		// Shadows
		case MENU_OPTIONS_SHADOWS:
			str = gameApp->mGameConfig->GetValue("graphics", "shadows", "on");
			if(change) {
				if(str == "on") str = "off";
				else if(str == "off") str = "on";
				gameApp->mGameConfig->SetValue("graphics", "shadows", str.c_str());
				mSceneMgr->setShadowTechnique((str == "off") ? SHADOWTYPE_NONE : SHADOWTYPE_STENCIL_ADDITIVE);
			}
			item->setCaption(itemText + ((str == "on") ? "ON" : "OFF"));
			break;

		// Dynamic lights
		case MENU_OPTIONS_DYN_LIGHTS:
			str = gameApp->mGameConfig->GetValue("graphics", "dynamic_lights", "on");
			if(change) {
				if(str == "on") str = "off";
				else if(str == "off") str = "on";
				gameApp->mGameConfig->SetValue("graphics", "dynamic_lights", str.c_str());
			}
			item->setCaption(itemText + ((str == "on") ? "ON" : "OFF"));
			break;

		// Dynamic light cast shadows
		case MENU_OPTIONS_DYN_LIGHT_SHADOWS:
			str = gameApp->mGameConfig->GetValue("graphics", "dynamic_light_shadows", "on");
			if(change) {
				if(str == "on") str = "off";
				else if(str == "off") str = "on";
				gameApp->mGameConfig->SetValue("graphics", "dynamic_light_shadows", str.c_str());
			}
			item->setCaption(itemText + ((str == "on") ? "ON" : "OFF"));
			break;

		// Sound volume
		case MENU_OPTIONS_SOUND_VOL:
			str = gameApp->mGameConfig->GetValue("audio", "volume", "1.0");
			vol = StringConverter::parseReal(str);
			if(change) {
				vol += 0.1f;
				if((int)(vol*100.0f) > 100) vol = 0.0f;
				str = StringConverter::toString(vol);
				gameApp->mGameConfig->SetValue("audio", "volume", str.c_str());

				// Play a test sound
				SoundSystem::getSingleton().setSoundVolume(vol);
				SoundSystem::getSingleton().playSound("mushroom1");
			}

			//ivol = (int)(100 * vol);
			sprintf(volstr, "%.0f%%", vol*100.0f);
			item->setCaption(itemText + String(volstr));
			break;

		// Music volume
		case MENU_OPTIONS_MUSIC_VOL:
			str = gameApp->mGameConfig->GetValue("audio", "music_volume", "0.5");
			vol = StringConverter::parseReal(str);
			if(change) {
				vol += 0.1f;
				if((int)(vol*100.0f) > 100) vol = 0.0f;
				str = StringConverter::toString(vol);
				gameApp->mGameConfig->SetValue("audio", "music_volume", str.c_str());

				SoundSystem::getSingleton().setMusicVolume(vol);
			}

			//ivol = (int)(100 * vol);
			sprintf(volstr, "%.0f%%", vol*100.0f);
			item->setCaption(itemText + String(volstr));
			break;
	}
}


// Prepare the options menu
void Menu::prepareOptionsItems() {
	for(int f=0; f<mNumItems; f++)
		updateOptionsItem(f);
}

// Show a menu
void Menu::show() {
	OverlayManager::getSingleton().getByName(mMenuName)->show();
	if(mMenuName == "MainMenu" || mMenuName == "HighScores")
		OverlayManager::getSingleton().getByName("Logo")->show();
}


// Hide a menu
void Menu::hide() {
	OverlayManager::getSingleton().getByName(mMenuName)->hide();
	if(mMenuName == "MainMenu" || mMenuName == "HighScores")
		OverlayManager::getSingleton().getByName("Logo")->hide();
}


// Scroll up
void Menu::scrollUp() {
	if(mNumItems == 0) return;

	dimItem(mSelection);
	mSelection--;
	if(mSelection < 0) mSelection = mNumItems-1;
	hilightItem(mSelection);

	SoundSystem::getSingleton().playSound("menumove");
}


// Scroll down
void Menu::scrollDown() {
	if(mNumItems == 0) return;

	dimItem(mSelection);
	mSelection++;
	if(mSelection > mNumItems-1) mSelection = 0;
	hilightItem(mSelection);

	SoundSystem::getSingleton().playSound("menumove");
}


// Handle a keypress
void Menu::handleKeyPress(const OIS::KeyEvent &e) {
	if(mEnteringName) {
		doEnterName(e);
		if(mEnteringName) return;
	}

	switch(e.key) {
		default: break;
		case OIS::KC_UP:
		case OIS::KC_LEFT:
			scrollUp();
			break;
		case OIS::KC_DOWN:
		case OIS::KC_RIGHT:
			scrollDown();
			break;
		case OIS::KC_ESCAPE:
			if(mMenuName == "MainMenu") break;
			if(mMenuName == "GameOptions") {
				// Restore the settings
				gameApp->mGameConfig->LoadFile(getConfigLocation(true).c_str());
				String str = gameApp->mGameConfig->GetValue("graphics", "bloom", "on");
				CompositorManager::getSingleton().setCompositorEnabled(gameApp->getRenderWindow()->getViewport(0), "Bloom", (str != "off"));

				str = gameApp->mGameConfig->GetValue("graphics", "shadows", "on");
				mSceneMgr->setShadowTechnique((str == "off") ? SHADOWTYPE_NONE : SHADOWTYPE_STENCIL_ADDITIVE);

				str = gameApp->mGameConfig->GetValue("audio", "volume", "1.0");
				SoundSystem::getSingleton().setSoundVolume(StringConverter::parseReal(str));

				str = gameApp->mGameConfig->GetValue("audio", "music_volume", "0.5");
				SoundSystem::getSingleton().setMusicVolume(StringConverter::parseReal(str));
			}
			// Return to main
			hide();
			setMenu("MainMenu", mPrevSelection);
			break;
		case OIS::KC_RETURN:
		case OIS::KC_SPACE:
		case OIS::KC_LCONTROL:
		case OIS::KC_RCONTROL:
		case OIS::KC_LSHIFT:
		case OIS::KC_RSHIFT:
			if(mMenuName == "MainMenu") mPrevSelection = mSelection;
			handleSelection();
			break;
	}
}


// Handle a selection
void Menu::handleSelection() {
	if(mMenuName == "MainMenu") {
		switch(mSelection) {
			case MENU_MAIN_START:	// Start a new game
				mNewState = STATE_GAME;
				hide();
				break;
			case MENU_MAIN_OPTIONS: // Options
				hide();
				setMenu("GameOptions");
				break;
			case MENU_MAIN_HIGHSCORES: // High scores
				hide();
				setMenu("HighScores");
				break;
			case MENU_MAIN_EXIT:	// Exit
				mNewState = STATE_QUIT;
				hide();
				break;
		}
	}
	else if(mMenuName == "GameOptions") {
		switch(mSelection) {
			case MENU_OPTIONS_BLOOM:
			case MENU_OPTIONS_SHADOWS:
			case MENU_OPTIONS_DYN_LIGHTS:
			case MENU_OPTIONS_DYN_LIGHT_SHADOWS:
			case MENU_OPTIONS_SOUND_VOL:
			case MENU_OPTIONS_MUSIC_VOL:
				updateOptionsItem(mSelection, true);
				break;
			case MENU_OPTIONS_CANCEL: {	// Cancel, back to main
				// Restore the settings
				gameApp->mGameConfig->LoadFile(getConfigLocation(true).c_str());
				String str = gameApp->mGameConfig->GetValue("graphics", "bloom", "on");
				CompositorManager::getSingleton().setCompositorEnabled(gameApp->getRenderWindow()->getViewport(0), "Bloom", (str != "off"));

				str = gameApp->mGameConfig->GetValue("graphics", "shadows", "on");
				mSceneMgr->setShadowTechnique((str == "off") ? SHADOWTYPE_NONE : SHADOWTYPE_STENCIL_ADDITIVE);

				str = gameApp->mGameConfig->GetValue("audio", "volume", "1.0");
				SoundSystem::getSingleton().setSoundVolume(StringConverter::parseReal(str));

				str = gameApp->mGameConfig->GetValue("audio", "music_volume", "0.5");
				SoundSystem::getSingleton().setMusicVolume(StringConverter::parseReal(str));
				
				// Return to main
				hide();
				setMenu("MainMenu", mPrevSelection);
				break;
				}

			case MENU_OPTIONS_ACCEPT:	// Save the settings, back to main
				// Save
				gameApp->mGameConfig->SaveFile(getConfigLocation(false).c_str());

				// Return to main
				hide();
				setMenu("MainMenu", mPrevSelection);
				break;
		}
	}
	else if(mMenuName == "HighScores") {
		// Return to main
		hide();
		setMenu("MainMenu", mPrevSelection);
	}
	else if(mMenuName == "TitleScreen") {
		// Return to main
		//hide();
		setMenu("MainMenu", mPrevSelection);
	}

	SoundSystem::getSingleton().playSound("menuselect");
}


// Highlight a menu item
void Menu::hilightItem(int i) {
	if(mNumItems == 0) return;

	TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mMenuName + StringConverter::toString(i+1)));
	item->setColourTop(ColourValue(0.5f,0.1f,0.0f));
	item->setColourBottom(ColourValue(1,0.5f,0.4f));
}


// Dim a menu item
void Menu::dimItem(int i) {
	if(mNumItems == 0) return;

	TextAreaOverlayElement *item = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mMenuName + StringConverter::toString(i+1)));
	item->setColourTop(ColourValue(0.5f,0.6f,0.7f,0.5f));
	item->setColourBottom(ColourValue(1,1,1,0.5f));
}


// Find the number of menu items for the current menu
int Menu::findMenuItems() {
	if(mMenuName == "MainMenu") return 4;
	if(mMenuName == "GameOptions") return 8;
	if(mMenuName == "HighScores") return 1;
	if(mMenuName == "TitleScreen") return 0;
	return 0;
}

