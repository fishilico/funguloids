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

// NOTICE: This file is written by Piet (thanks! :), <funguloids@superpiet.de>,
// for the Linux version of Funguloids.

#ifndef OALSOUNDSYSTEM_H
#define OALSOUNDSYSTEM_H

#include "soundsystem.h"

#ifdef SOUND_SYSTEM_OPENAL

#include <Ogre.h>
#include <OgreSingleton.h>
#include <map>
#include <list>
#include <AL/al.h>
#include <AL/alut.h>

#include "openalchannelgroup.h"
#include "streamplayer.h"

using namespace Ogre;
using namespace std;


typedef map<String, ALuint> SoundMapType;

// Number of independent channels for sound fx
const int NUM_SOUND_CHANNELS = 32;

// OpenALSoundSystem
class OpenALSoundSystem : public Singleton<OpenALSoundSystem> {
public:
	OpenALSoundSystem(SceneManager *mgr);
	~OpenALSoundSystem();

	void setListenerPosition( float x, float y, float z, float angle );

	void update();
	void loadSound(const String &file, Real freqVar = 0, bool looped = false);
	OpenALChannel* playSound(const String &file, Real pan = 0);
	OpenALChannel* playLoopedSound(const String &file);
	void setSoundVolume(Real vol);

	void playMusic(const std::string& file);
	void setMusicVolume(Real vol);
	void playNextSong() { mPlayNextSong = true; }

	static OpenALSoundSystem& getSingleton();
	static OpenALSoundSystem* getSingletonPtr();
protected:
	int initialise( unsigned int deviceIndex = 0 );
	void shutdown();
	int addChannel();
	void stopAllChannels();

	SoundMapType mSounds;

private:
	SceneManager *mSceneMgr;			// Ogre scene manager
	bool mSoundDisabled;				// Is the sound output disabled?
	bool mPlayNextSong;					// Should we change the song?

	int addChannelgroup( unsigned int numChannels );

	// internal OpenAL stuff
	std::string lastALError( ALenum err );
	std::string lastALUTError( ALenum err );
	int getDevices();
	ALCdevice* mDevice;
	ALCcontext* mContext;
	std::vector<std::string> mDevices;
	int mIndexDefaultDevice;

	// listeners position in the world (a vector), it's velocity (unused, set to 0,0,0)
	ALfloat mListenerPosition[3];
	ALfloat mListenerVelocity[3];
	ALfloat mListenerOrientation[6];

	// vector containing all samples/buffer/sounds (whatever you want to name it)
	std::vector<ALuint> mSamples;

	// a ChannelGroup containing NUM_SOUND_CHANNELS OpenALChannels for
	// sound playback
	ChannelGroup* mSoundChannels;

	typedef std::vector<OpenALChannel*> ChannelVec;
	ChannelVec mChannel; //!< This vector contains all allocated Channels

	typedef std::vector<ChannelGroup*> ChannelGroupVec;
	ChannelGroupVec mChannelGroup; //!< This vector contains all ChannelGroups

//	bool mMusic;
	StreamPlayer* mStreamPlayer;

};

typedef OpenALChannel SoundChannel;
typedef OpenALSoundSystem SoundSystem;

#endif

#endif
