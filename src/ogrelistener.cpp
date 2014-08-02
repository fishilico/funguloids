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
#include "objectsystem.h"
#include "game.h"
#include <OgreOverlayManager.h>


// After a rendered frame
bool OgreAppFrameListener::frameEnded(const FrameEvent &evt) {
	// Update the stats (FPS, etc.)
	if(mStatsOn)
		updateStats();
	return true;
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
	}
	catch(...) {
		// ignore
	}
}


// Register the compositors
void OgreAppFrameListener::registerCompositors() {
	Viewport *vp = mApp->getRenderWindow()->getViewport(0);
	CompositorManager::getSingleton().addCompositor(vp, "Bloom");
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
	mRenderMode = 0;
	mScreenShots = 0;

	// Initialise input
	size_t windowHnd = 0;
	win->getCustomAttribute("WINDOW", &windowHnd);
	mInput = new InputHandler(this, windowHnd);

	windowResized(mWindow);
	WindowEventUtilities::addWindowEventListener(mWindow, this);


	// Show debug info?
	if(strcmp(GameApplication::mGameConfig->GetValue("graphics", "debug_info", "off"),"on") == 0) {
		showDebugOverlay(true);
		mStatsOn = true;
	}

	registerCompositors();

}


// Unattach OIS before window shutdown (very important under Linux)
void OgreAppFrameListener::windowClosed(RenderWindow *rw) {
	if(rw == mWindow) {
		if(mInput) {
			delete mInput;
			mInput = 0;
		}
	}
}


// Destructor
OgreAppFrameListener::~OgreAppFrameListener() {
	// Remove ourself as a Window listener
	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
}
