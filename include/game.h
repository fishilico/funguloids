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

#ifndef GAME_H
#define GAME_H

#include "ogreapp.h"
#include "menu.h"
#include "SimpleIni.h"

// Playfield size
extern Real playfieldWidth;
extern Real playfieldHeight;


// Game states
enum {
	STATE_MENU = 0,
	STATE_GAME,
	STATE_QUIT
};


#define SETTINGS_FILE	"gamesettings.cfg"

// Helper function which returns suitable path for the
// config file. It first checks the user's home directory,
// (in Linux only) and if that fails it uses the default directory.
String getConfigLocation(bool reading);


class Player;
class GameCamera;
class Menu;

// Game class
class GameApplication : public OgreApplication, public RenderTargetListener {
public:
	GameApplication() {
		// Load the game settings
		//mGameConfig = new ConfigFile();
		mGameConfig = new CSimpleIniA();
		mGameConfig->LoadFile(getConfigLocation(true).c_str());
		//mGameConfig->load("gamesettings.cfg");
	}

	virtual ~GameApplication();

	const String getTitle() const;

	//static ConfigFile *mGameConfig;
	static CSimpleIniA *mGameConfig;

	void startNewGame();
	void endGame();
	void loadLevel(const String &level);
	void clearLevel();
	void updateSpecialLevelTransitionEffect(Real alpha);
	void updateBallWormGlowEffect(Real alpha);
	int getState() const { return mGameState; }
	void setEndGame();
	bool hasGameEnded() const { return mEndGame; }
	void menuKeyPress(const OIS::KeyEvent &e) {
		mMenu->handleKeyPress(e);
	}
	Menu *getMenu() { return mMenu; }
	void quit() { mFrameListener->quit(); }

protected:
	void createFrameListener() {
        mFrameListener = new OgreAppFrameListener(this, mWindow, mCamera, mGameCamera, mSceneMgr, mPlayer);
        mRoot->addFrameListener(mFrameListener);
	}

	void createSceneManager() {
		mSceneMgr = mRoot->createSceneManager("OctreeSceneManager");
	}

	void createCamera();
	void createScene();

private:
	Player *mPlayer;
	GameCamera *mGameCamera;
	Overlay *mSpecialLevelTransition;
	Overlay *mBallWormGlowEffect;
	int mGameState;			// What state are we in? (game, main menu, etc..)
	bool mEndGame;
	Menu *mMenu;
};


// Global game application pointer (I know, I know.. Bad, baad me.. :P)
extern GameApplication *gameApp;


#endif
