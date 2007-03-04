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

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <Ogre.h>
#include <OgreSingleton.h>
#include <fmod.hpp>
#include <fmod_errors.h>
#include <map>

using namespace Ogre;
using namespace std;


typedef map<String, FMOD::Sound*> SoundMapType;

// SoundSystem
class SoundSystem : public Singleton<SoundSystem> {
private:
	FMOD::System *mSystem;				// FMOD System
	FMOD::Sound *mMusic;				// Background music stream
	FMOD::ChannelGroup *mSoundChannels;	// Sound channels group
	FMOD::Channel *mMusicChannel;		// Music channel
	SceneManager *mSceneMgr;			// Ogre scene manager

	SoundMapType mSounds;				// Sounds
	bool mSoundDisabled;				// Is the sound output disabled?
	bool mPlayNextSong;					// Should we change the song?

public:
	SoundSystem(SceneManager *mgr);
	~SoundSystem();

	void update();
	void playMusic(const char *file);
	void loadSound(const String &file, Real freqVar = 0, bool looped = false);
	FMOD::Channel *playSound(const String &file, Real pan = 0);
	FMOD::Channel *playLoopedSound(const String &file);
	void setMusicVolume(Real vol);
	void setSoundVolume(Real vol);
	void playNextSong() { mPlayNextSong = true; }

	static SoundSystem &getSingleton();
	static SoundSystem *getSingletonPtr();
};


#endif
