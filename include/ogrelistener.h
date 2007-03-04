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

#ifndef OGRELISTENER_H
#define OGRELISTENER_H

#include <Ogre.h>
#include <OgreEventListeners.h>
#include <OgreKeyEvent.h>
#include <OgreStringConverter.h>
#include <OgreException.h>

using namespace Ogre;


class OgreApplication;
class Player;
class GameCamera;

// Frame listener
class OgreAppFrameListener : public FrameListener, public KeyListener, public MouseMotionListener, public MouseListener {
public:
	OgreAppFrameListener(OgreApplication *app, RenderWindow *win, Camera *cam, GameCamera *gamecam, SceneManager *sceneMgr, Player *player);
	virtual ~OgreAppFrameListener() {
		if(mEventProcessor)
			delete mEventProcessor;
	}


	// MouseMotion
	void mouseMoved(MouseEvent *e);
	void mouseDragged(MouseEvent *e);

	// MouseListener
	void mouseClicked(MouseEvent *e);
	void mouseEntered(MouseEvent *e) { }
	void mouseExited(MouseEvent *e) { }
	void mousePressed(MouseEvent *e);
	void mouseReleased(MouseEvent *e);

	// KeyListener
	void keyClicked(KeyEvent *e) { }
	void keyPressed(KeyEvent *e);
	void keyReleased(KeyEvent *e);

	void quit() { mQuit = true; }

	void doGame(Real delta);
	void doMenu(Real delta, const bool keyDown[]);

	bool frameStarted(const FrameEvent &evt);
	bool frameEnded(const FrameEvent &evt);

	void saveScreenshot();
	void registerCompositors();

	void showDebugOverlay(bool show) {
		if(mDebugOverlay) {
			if(show)
				mDebugOverlay->show();
			else
				mDebugOverlay->hide();
		}
	}

	bool isLMouseDown() const { return mLMouseDown; }
	bool isRMouseDown() const { return mRMouseDown; }
	bool isMMouseDown() const { return mMMouseDown; }

	void setPlayer(Player *player) { mPlayer = player; }

protected:
	EventProcessor *mEventProcessor;
	InputReader *mInputDevice;
	RenderWindow *mWindow;
	Camera *mCamera;
	SceneManager *mSceneMgr;

	Player *mPlayer;
	GameCamera *mGameCamera;
	bool mKeyDown[256];

	bool mLMouseDown, mRMouseDown, mMMouseDown;

	int mRenderMode;
	int mScreenShots;
	bool mStatsOn;
	bool mShowOctree;
	bool mQuit;
	Overlay *mDebugOverlay;
	Overlay *mDisplayOverlay;
	Overlay *mMenuOverlay;

	OgreApplication *mApp;


	void updateStats();
};

#endif
