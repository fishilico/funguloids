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

#include "ogreapp.h"
#include "objectsystem.h"
#include "effects.h"
#include "scriptsystem.h"
#include "soundsystem.h"
#include "mpakogre.h"
#include <OgreOverlaySystem.h>

#if(OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
extern HINSTANCE hInstance;	// from main.cpp
#endif


#if(OGRE_PLATFORM == OGRE_PLATFORM_LINUX)
	extern String getFunguloidsDir();
#endif

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

// Defines for the different paths
#ifndef OGRE_PLUGINS_AND_RESOURCES_PATH		// This path needs read only access
  #define OGRE_PLUGINS_AND_RESOURCES_PATH ""
#endif
#if(OGRE_PLATFORM == OGRE_PLATFORM_LINUX)
  #ifndef OGRE_CONFIG_AND_LOG_PATH		// This path needs rw access
    #define OGRE_CONFIG_AND_LOG_PATH getFunguloidsDir()
  #endif
#endif
#if(OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
  #ifndef OGRE_CONFIG_AND_LOG_PATH
    #define OGRE_CONFIG_AND_LOG_PATH String("")
  #endif 
#endif



// Setup
bool OgreApplication::setup() {
	// Create Ogre root
	mRoot = new Root(
			String(OGRE_PLUGINS_AND_RESOURCES_PATH) + "plugins.cfg",
			OGRE_CONFIG_AND_LOG_PATH + "ogre.cfg",
			OGRE_CONFIG_AND_LOG_PATH + "Ogre.log"
			);

	// Random seed
	srand(time(NULL));

	// Initialize the overlay system
	OverlaySystem *overlaySys = new OverlaySystem();

	// Add the MPK archive support
	mMPakFactory = new MPakArchiveFactory();
	ArchiveManager::getSingleton().addArchiveFactory(mMPakFactory);

	// Load resource paths from config file
	ConfigFile cf;
	cf.load(String(OGRE_PLUGINS_AND_RESOURCES_PATH) + "resources.cfg");

	// Parse the resources.cfg
	ConfigFile::SectionIterator seci = cf.getSectionIterator();

	String secName, typeName, archName;
	while(seci.hasMoreElements()) {
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for(i = settings->begin(); i != settings->end(); ++i) {
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}


	// Get the configuration
	if(!mRoot->restoreConfig())
		if(!mRoot->showConfigDialog())
			return false;

	// Initialise the system
	mWindow = mRoot->initialise(true, getTitle().c_str());

	// Set the icon (on Win32)
#if(OGRE_PLATFORM == OGRE_PLATFORM_WIN32)
	HWND hWnd;
	mWindow->getCustomAttribute("WINDOW", &hWnd);
	SetClassLong(hWnd, GCL_HICON, (LONG)LoadIcon(hInstance, "ICON1"));
#endif

	// Create the SceneManager
	createSceneManager();
	mSceneMgr->addRenderQueueListener(overlaySys);

	// Set up the scene
	createCamera();
	createViewport();

	// Set default mipmap level
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Initialise resources, parse scripts etc
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	// Initialise the sound system
	new SoundSystem(mSceneMgr);

	// Create the scene
	createScene();

	// Create the frame listener
	createFrameListener();
	return true;
}

// Destructor
OgreApplication::~OgreApplication() {
	if(ObjectSystem::getSingletonPtr())
		delete ObjectSystem::getSingletonPtr();

	destroyEffectLights(mSceneMgr);

	if(ScriptSystem::getSingletonPtr())
		delete ScriptSystem::getSingletonPtr();

	if(SoundSystem::getSingletonPtr())
		delete SoundSystem::getSingletonPtr();

	if(mFrameListener)
		delete mFrameListener;

	if(mRoot)
		delete mRoot;

	if(mMPakFactory)
		delete mMPakFactory;
}

