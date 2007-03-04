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

#include "ogrelistener.h"
#include "ogreapp.h"
#include "base.h"
#include "objectsystem.h"
#include "soundsystem.h"
#include "playlist.h"
#include "game.h"


// After a rendered frame
bool OgreAppFrameListener::frameEnded(const FrameEvent &evt) {
	// Update the stats (FPS, etc.)
	if(mStatsOn)
		updateStats();
	return true;
}


// Mouse movement
void OgreAppFrameListener::mouseMoved(MouseEvent *e) {
	// Send it to the menu if necessary
	if(gameApp->getState() != STATE_GAME) {
		Real zmov = e->getRelZ();
		if(zmov < 0)
			gameApp->getMenu()->scrollDown();
		else if(zmov > 0)
			gameApp->getMenu()->scrollUp();
	}
	e->consume();
}

// Mouse dragging
void OgreAppFrameListener::mouseDragged(MouseEvent *e) {
	// Send it to the menu if necessary
	if(gameApp->getState() != STATE_GAME) {
		Real zmov = e->getRelZ();
		if(zmov < 0)
			gameApp->getMenu()->scrollDown();
		else if(zmov > 0)
			gameApp->getMenu()->scrollUp();
	}
	e->consume();
}

// MouseListener
void OgreAppFrameListener::mousePressed(MouseEvent *e) {
	// Send it to the menu if necessary
	if(!mLMouseDown && gameApp->getState() != STATE_GAME) {
		// Send a simulated keypress
		KeyEvent k(NULL, 0, KC_RETURN, 0, 0);
		gameApp->getMenu()->handleKeyPress(&k);
	}

	// Left mouse button down
	if(e->getButtonID() & InputEvent::BUTTON0_MASK) {
		mLMouseDown = true;
	}

	// Right mouse button down
	else if(e->getButtonID() & InputEvent::BUTTON1_MASK) {
		mRMouseDown = true;
	}

	// Middle mouse button down
	else if(e->getButtonID() & InputEvent::BUTTON2_MASK) {
		mMMouseDown = true;
	}

	e->consume();
}

void OgreAppFrameListener::mouseReleased(MouseEvent *e) {
	// Left mouse button up
	if(e->getButtonID() & InputEvent::BUTTON0_MASK) {
		mLMouseDown = false;
	}

	// Right mouse button up
	else if(e->getButtonID() & InputEvent::BUTTON1_MASK) {
		mRMouseDown = false;
	}

	// Middle mouse button up
	else if(e->getButtonID() & InputEvent::BUTTON2_MASK) {
		mMMouseDown = false;
	}

	e->consume();
}

void OgreAppFrameListener::mouseClicked(MouseEvent *e) {
	e->consume();
}


// KeyListener
void OgreAppFrameListener::keyPressed(KeyEvent *e) {
	// Quit?
/*	if(e->getKey() == KC_ESCAPE) {
		mQuit = true;
		e->consume();
		return;
	}
*/
	mKeyDown[e->getKey()] = true;

	// Handle the key
	switch(e->getKey()) {
		case KC_F1:
			// Play a new track
			SoundSystem::getSingleton().playMusic(getNextSong().c_str());
			break;

		case KC_F:
			if(!e->isControlDown()) break;
			// Toggle the stats display
			mStatsOn = !mStatsOn;
			showDebugOverlay(mStatsOn);
			break;

		case KC_SYSRQ:
			// Take a screenshot
			saveScreenshot();
			mApp->getRoot()->clearEventTimes();
			break;
	}

	// Send it to the menu if necessary
	if(gameApp->getState() != STATE_GAME)
		gameApp->menuKeyPress(e);

	e->consume();
}

void OgreAppFrameListener::keyReleased(KeyEvent *e) {
	mKeyDown[e->getKey()] = false;
	e->consume();
}



// Update the debug overlay
void OgreAppFrameListener::updateStats() {
	try {
		OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
		OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
		OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
		OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

		const RenderTarget::FrameStats &stats = mWindow->getStatistics();

		guiAvg->setCaption(String("Average FPS: ") + StringConverter::toString(stats.avgFPS));
		guiCurr->setCaption(String("Current FPS: ") + StringConverter::toString(stats.lastFPS));
		guiBest->setCaption(String("Best FPS: ") + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		guiWorst->setCaption(String("Worst FPS: ") + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		guiTris->setCaption(String("Triangle count: ") + StringConverter::toString(stats.triangleCount));

		OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		guiDbg->setCaption(mWindow->getDebugText());
	}
	catch(...) {
		// ignore
	}
}


// Register the compositors
void OgreAppFrameListener::registerCompositors() {
	Viewport *vp = mApp->getRenderWindow()->getViewport(0);
	CompositorInstance *instance = CompositorManager::getSingleton().addCompositor(vp, "Bloom");
	String bloom = GameApplication::mGameConfig->GetValue("graphics", "bloom", "on");
	CompositorManager::getSingleton().setCompositorEnabled(vp, "Bloom", bloom.compare("off") != 0);

	MaterialPtr mat = MaterialManager::getSingleton().getByName("Ogre/Compositor/BloomBlend");
	Real effect;
	String str = gameApp->mGameConfig->GetValue("graphics", "bloom_amount", "1.0");
	effect = StringConverter::parseReal(str);
	effect *= 0.5f;
	mat->setSelfIllumination(effect, effect, effect);

	CompositorManager::getSingleton().addCompositor(vp, "B&W");
	CompositorManager::getSingleton().setCompositorEnabled(vp, "B&W", true);
}


// Save a screen shot
void OgreAppFrameListener::saveScreenshot() {
	int count = 99;
	while(count--) {
		// Check if the file exists
		char tmp[32];
		sprintf(tmp, "shot_%02d.png", mScreenShots++);
		FILE *check = fopen(tmp, "r");
		if(!check) {
			mWindow->writeContentsToFile(tmp);
			LogManager::getSingleton().logMessage("Wrote screenshot to " + String(tmp) + ".");
			break;
		}
		else
			fclose(check);
	}
}


// Constructor
OgreAppFrameListener::OgreAppFrameListener(OgreApplication *app, RenderWindow *win, Camera *cam, GameCamera *gamecam, SceneManager *sceneMgr, Player *player) {
	mDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	mDisplayOverlay = OverlayManager::getSingleton().getByName("DisplayOverlay");
	mMenuOverlay = OverlayManager::getSingleton().getByName("MainMenu");

	mApp = app;
	mSceneMgr = sceneMgr;
	mCamera = cam;
	mWindow = win;
	mStatsOn = false;
	mShowOctree = true;
	mQuit = false;
	mPlayer = player;
	mGameCamera = gamecam;
	mLMouseDown = mRMouseDown = mMMouseDown = false;
	mRenderMode = 0;
	mScreenShots = 0;

	mEventProcessor = new EventProcessor();
	mEventProcessor->initialise(mWindow);
	mEventProcessor->addMouseListener(this);
	mEventProcessor->addMouseMotionListener(this);
	mEventProcessor->addKeyListener(this);
	mEventProcessor->startProcessingEvents();
	mInputDevice = mEventProcessor->getInputReader();

	if(GameApplication::mGameConfig->GetValue("graphics", "debug_info", "off") == "on") {
		showDebugOverlay(true);
		mStatsOn = true;
	}

	registerCompositors();

	for(int i=0; i<256; i++)
		mKeyDown[i] = false;
}
