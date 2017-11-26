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

#ifndef OGREAPP_H
#define OGREAPP_H

#include <Ogre.h>
#include <OgreConfigFile.h>
#include "ogrelistener.h"

using namespace Ogre;


// Base class for an Ogre application.
class OgreApplication {
public:
	OgreApplication() {
		mFrameListener = 0;
		mRoot = 0;
	}
	virtual ~OgreApplication();


	// Start the application
	virtual void start() {
		if(!setup())
			return;

		mRoot->startRendering();

		// Clean up
		destroyScene();
	}


	// Returns the window title
	virtual const String getTitle() const {
		return "OgreApplication";
	}

	Root *getRoot() const { return mRoot; }
	RenderWindow *getRenderWindow() const { return mWindow; }

protected:
	Root *mRoot;
	Camera *mCamera;
	SceneNode* mCamNode;
	SceneManager *mSceneMgr;
	RenderWindow *mWindow;
	OgreAppFrameListener *mFrameListener;
	class MPakArchiveFactory *mMPakFactory;

	// Sets up the application, returns false if the user cancels
	virtual bool setup();

	// Create the scene
	virtual void createScene() = 0;

	// Destroy the scene
    virtual void destroyScene() { }

	// Create the scene manager
	virtual void createSceneManager() {
		mSceneMgr = mRoot->createSceneManager(ST_GENERIC);
	}

	// Create the frame listener
	virtual void createFrameListener() {
        mFrameListener = new OgreAppFrameListener(this, mWindow, mCamera, NULL, mSceneMgr, NULL);
        mRoot->addFrameListener(mFrameListener);
	}

	// Create the camera
	virtual void createCamera() {
		mCamNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mCamera = mSceneMgr->createCamera("PlayerCam");
		mCamera->setNearClipDistance(5);
		mCamNode->attachObject(mCamera);
		mCamNode->setPosition(Vector3(0,0,500));
		mCamera->lookAt(Vector3(0,0,-300));
	}

    // Create the viewport
	virtual void createViewport() {
		Viewport *vp = mWindow->addViewport(mCamera);
		vp->setBackgroundColour(ColourValue(0,0,0));
		mCamera->setAspectRatio(Real(vp->getActualWidth()) / Real(vp->getActualHeight()));
	}

};


#endif
