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

#include "game.h"
#include "player.h"
#include "objectsystem.h"
#include "gamecamera.h"
#include "base.h"
#include "effects.h"
#include "bground.h"
#include "scriptsystem.h"
#include "soundsystem.h"
#include "hiscore.h"


// Playfield size (which is really read from the level script..)
Real playfieldWidth = 150.0f;
Real playfieldHeight = 116.0f;

//ConfigFile *GameApplication::mGameConfig = NULL;
CSimpleIniA *GameApplication::mGameConfig = NULL;

// Global game application pointer (I know, I know.. Bad, baad me.. :P)
GameApplication *gameApp;



// Prepare the frame to render
bool OgreAppFrameListener::frameStarted(const FrameEvent &evt) {
	if(mWindow->isClosed() || mQuit)
		return false;

	// Do the game / menu
	switch(gameApp->getState()) {
		case STATE_MENU:
			doMenu(evt.timeSinceLastFrame, mKeyDown); break;
		case STATE_GAME:
			doGame(evt.timeSinceLastFrame); break;
	}

	// Update the sound system
	SoundSystem::getSingleton().update();

	if(gameApp->hasGameEnded())
		gameApp->endGame();
	return true;
}


// Update the special level transition effect
void GameApplication::updateSpecialLevelTransitionEffect(Real alpha) {
	if(alpha > 0) {
		if(!mSpecialLevelTransition->isVisible())
			mSpecialLevelTransition->show();
		MaterialPtr mat = MaterialManager::getSingleton().getByName("SpecialLevelTransition");
		mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, ColourValue(alpha,alpha,alpha), ColourValue(alpha,alpha,alpha));
		mat->getTechnique(0)->getPass(1)->getTextureUnitState(1)->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, ColourValue(alpha,alpha,alpha), ColourValue(alpha,alpha,alpha));
	}
	else
		mSpecialLevelTransition->hide();
}


// Update the ball worm glow effect
void GameApplication::updateBallWormGlowEffect(Real alpha) {
	if(alpha > 0) {
		if(!mBallWormGlowEffect->isVisible())
			mBallWormGlowEffect->show();
		MaterialPtr mat = MaterialManager::getSingleton().getByName("BallWormGlow");
		mat->getTechnique(0)->getPass(0)->getTextureUnitState(1)->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, ColourValue(alpha,alpha,alpha), ColourValue(alpha,alpha,alpha));
		mat->getTechnique(0)->getPass(1)->getTextureUnitState(1)->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, ColourValue(alpha,alpha,alpha), ColourValue(alpha,alpha,alpha));
	}
	else
		mBallWormGlowEffect->hide();
}


// Update the game logic
void OgreAppFrameListener::doGame(Real delta) {
	// Update all the moving objects (enemies, ...)
	if(!mPlayer->isConfirmQuit()) {
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
	}

	if(mPlayer->isConfirmQuit()) delta = 0;

	// Move the player
	mPlayer->move(delta, mKeyDown);


	// Move the camera
	mGameCamera->followTarget(delta, (mPlayer->isDead() ? 2.0f : 5.5f));
}


// Prepare the scene
void GameApplication::createScene() {
	gameApp = this;
	mGameState = STATE_MENU;
	mEndGame = false;
	mPlayer = NULL;

	// Special level transition overlay
	mSpecialLevelTransition = OverlayManager::getSingleton().getByName("SpecialLevelTransition");
	mBallWormGlowEffect = OverlayManager::getSingleton().getByName("BallWormGlow");

	// Script system
	new ScriptSystem(mSceneMgr, "startup.lua");

	// Set up the shadows
	String shadows = GameApplication::mGameConfig->GetValue("graphics", "shadows", "on");
	if(shadows.compare("off") != 0) {
		mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_ADDITIVE);
		mSceneMgr->setShadowColour(ColourValue(0.5f,0.5f,0.5f));
	}

	// Load the sounds
	SoundSystem *ss = SoundSystem::getSingletonPtr();
	ss->loadSound("mushroom1", 400);
	ss->loadSound("mushroom2", 400);
	ss->loadSound("leave_base", 400);
	ss->loadSound("score", 50);
	ss->loadSound("base_hanging", 0, true);
	ss->loadSound("death", 400);
	ss->loadSound("base_enter", 400);
	ss->loadSound("flight1", 0);
	for(int i=1; i<12; i++)
		ss->loadSound("flight" + StringConverter::toString(i+1), 200);
	ss->loadSound("whirler", 200);
	ss->loadSound("change_base", 400);
	ss->loadSound("menumove", 400);
	ss->loadSound("menuselect", 400);
	ss->loadSound("typing", 400);
	ss->loadSound("balls", 50);

	// Create the object system
	new ObjectSystem(mSceneMgr);

	// Create the effect lights pool
	createEffectLights(mSceneMgr);

	// Create the menus and go to the title screen
	mMenu = new Menu(mSceneMgr);
	mMenu->setMenu("TitleScreen");
}


// Start a new game
void GameApplication::startNewGame() {
	clearLevel();

	if(mPlayer) {
		delete mPlayer;
		mPlayer = NULL;
	}


	mGameCamera->resetRotation();
	mEndGame = false;
	mGameState = STATE_GAME;
	OverlayManager::getSingleton().getByName("DisplayOverlay")->show();

	// Create the player
	mPlayer = new Player("Player", mSceneMgr, "Drop.mesh", Vector3(0,0,0), mGameCamera, false);
	mPlayer->setCastShadows(false);
	mGameCamera->setTarget(mPlayer->getSceneNode());
	ScriptSystem::getSingleton().setPlayer(mPlayer);
	mFrameListener->setPlayer(mPlayer);

	ScriptSystem::getSingleton().executeString("NewGame();");

	// Load the first level
	loadLevel("level1.lua");
}


// End the current game
void GameApplication::endGame() {
	OverlayManager::getSingleton().getByName("DisplayOverlay")->hide();
	mGameState = STATE_MENU;
	mEndGame = false;

	Overlay *scroller = OverlayManager::getSingleton().getByName("MenuScroller");
	scroller->setScroll(0,0);

	// Check if the player qualifies for the high scores
	if(mPlayer->isDead()) {
		hiscoreList.load(HISCORE_FILE);
		int place = hiscoreList.addName("zzz", mPlayer->getScore());
		if(place != -1) {
			mMenu->setMenu("HighScores");
			mMenu->enterName(place);
		}
		else
			mMenu->setMenu("MainMenu");
	}
	else
		mMenu->setMenu("MainMenu");
}


// Load a level
void GameApplication::loadLevel(const String &level) {
	// Load the level script
	ScriptSystem::getSingleton().executeScript(level.c_str());

	// Get the level dimensions
	playfieldWidth = ScriptSystem::getSingleton().getScriptNumber("playfieldWidth");
	playfieldHeight = ScriptSystem::getSingleton().getScriptNumber("playfieldHeight");


	// Create the background
	ScriptSystem::getSingleton().executeString("BackgroundCreate();");
	createBackground(mSceneMgr);


	// Create the lights
	ScriptSystem::getSingleton().executeString("LightsCreate();");


	// Level borders
/*	ManualObject *manual = mSceneMgr->createManualObject("viivat");
	manual->begin("BaseWhiteNoLighting", RenderOperation::OT_LINE_STRIP);
	manual->position(-playfieldWidth, -playfieldHeight, 0);
	manual->position(-playfieldWidth,  playfieldHeight, 0);
	manual->position( playfieldWidth,  playfieldHeight, 0);
	manual->position( playfieldWidth, -playfieldHeight, 0);
	manual->index(0);
	manual->index(1);
	manual->index(2);
	manual->index(3);
	manual->index(0);
	manual->end();
	mSceneMgr->getRootSceneNode()->createChildSceneNode("LevelBorders")->attachObject(manual);
*/

	// Create the objects
	ScriptSystem::getSingleton().executeString("LevelCreate();");

	// Position the player to the home base
	if(mPlayer) {
		Vector3 pos;
		MovingObject *base = ObjectSystem::getSingleton().findObject(ScriptSystem::getSingleton().getScriptString("playerHomeBase"));
		pos = base->getPosition();
		pos.z = 0;
		mPlayer->moveTo(pos);
		mPlayer->setLastBase(static_cast<Base*>(base));
	}


	// Set the bloom
	MaterialPtr mat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend");
	Real effect;
	String st = mGameConfig->GetValue("graphics", "bloom_amount", "1.0");
	effect = StringConverter::parseReal(st);
	if(level != "special_level.lua")
		effect *= 0.5f;
	mat->setSelfIllumination(effect, effect, effect);

	mRoot->clearEventTimes();
}


// Clear the level
void GameApplication::clearLevel() {
	try {
		destroyBackground(mSceneMgr);
		ScriptSystem::getSingleton().destroyScriptLights();
		ObjectSystem::getSingleton().destroyObjects();
	}
	catch(...) {
		// Ignore..
	}
}


// Set the end game
void GameApplication::setEndGame() {
	mEndGame = true;
	updateBallWormGlowEffect(0);
	updateSpecialLevelTransitionEffect(0);
}


void GameApplication::createCamera() {
	mCamera = mSceneMgr->createCamera("MainCam");
	mCamera->setPosition(Vector3(0,0,115));
	mCamera->lookAt(Vector3(0,0,0));
	mCamera->setNearClipDistance(10);

	mGameCamera = new GameCamera(mCamera);
}


GameApplication::~GameApplication() {
	clearLevel();
	if(mPlayer)
		delete mPlayer;

	if(mMenu)
		delete mMenu;

	if(mGameCamera)
		delete mGameCamera;

	if(mGameConfig)
		delete mGameConfig;
}


// Game title
const String GameApplication::getTitle() const {
	return "Those Funny Funguloids!";
}

