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

#include "soundsystem.h"

#ifdef SOUND_SYSTEM_FMOD

#include "game.h"
#include "fmodexsoundsystem.h"
#include "playlist.h"
#include "mpakogre.h"

template<> FMODExSoundSystem* Singleton<FMODExSoundSystem>::ms_Singleton = 0;


// Checks for FMOD error and prints an error message
bool errorCheck(FMOD_RESULT result) {
	if(result != FMOD_OK) {
		LogManager::getSingleton().logMessage("FMOD Error: " + String(FMOD_ErrorString(result)));
		return true;
	}
	return false;
}


// Music end callback
FMOD_RESULT F_CALLBACK endCallBack(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, int command, unsigned int commandData1, unsigned int commandData2) {
	// Play another song
	//FMODExSoundSystem::getSingleton().playMusic(getNextSong());
	FMODExSoundSystem::getSingleton().playNextSong();
	return FMOD_OK;
}

// File system callbacks
// (fileName is a pointer to a SoundResource)
FMOD_RESULT F_CALLBACK fileOpen(const char *fileName, int unicode, unsigned int *fileSize, void **handle, void **userData) {
	SoundResource *soundResource;
	assert(fileName);

	soundResource = (SoundResource*)fileName;
	assert(soundResource->fileArchive);

	*handle = static_cast<void*>(soundResource);
	*userData = NULL;

	soundResource->streamPtr = soundResource->fileArchive->open(soundResource->fileName);
	if(soundResource->streamPtr.isNull()) {
		*fileSize = 0;
		return FMOD_ERR_FILE_NOTFOUND;
	}

	*fileSize = (unsigned int)soundResource->streamPtr->size();
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fileClose(void *handle, void *userdata) {
	if(!handle)
		return FMOD_ERR_INVALID_PARAM;

	SoundResource *soundResource;
	soundResource = static_cast<SoundResource*>(handle);
	soundResource->streamPtr->close();

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fileRead(void *handle, void *buffer, unsigned int sizeBytes, unsigned int *bytesRead, void *userData) {
	SoundResource *soundResource;

	soundResource = static_cast<SoundResource*>(handle);
	*bytesRead = (unsigned int)soundResource->streamPtr->read(buffer, (size_t)sizeBytes);
	if(*bytesRead == 0)
		return FMOD_ERR_FILE_EOF;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK fileSeek(void *handle, unsigned int pos, void *userdata) {
	SoundResource *soundResource;

	soundResource = static_cast<SoundResource*>(handle);
	soundResource->streamPtr->seek((size_t)pos);
	return FMOD_OK;
}



// Load a sound (with specified frequency variation)
void FMODExSoundSystem::loadSound(const String &file, Real freqVar, bool looped) {
	if(mSoundDisabled) return;

	//String soundFile = "media/sounds/" + file + ".wav";
	String soundFile = file + ".wav";
	LogManager::getSingleton().logMessage("Loading " + soundFile + "..");

	SoundResource *sres = new SoundResource();
	SoundLocator *soundLocator = static_cast<SoundLocator*>(ResourceGroupManager::getSingletonPtr());
	sres->fileName = soundFile;
	sres->fileArchive = soundLocator->findSound(soundFile);
	if(!sres->fileArchive) {
		LogManager::getSingleton().logMessage("Could not find " + soundFile + "!");
		return;
	}


	FMOD::Sound *sound;
	FMOD_MODE flags = FMOD_2D|FMOD_SOFTWARE;
	if(looped) flags |= FMOD_LOOP_NORMAL;
	else flags |= FMOD_LOOP_OFF;

	//FMOD_RESULT result = mSystem->createSound(soundFile.c_str(), flags, NULL, &sound);
	FMOD_RESULT result = mSystem->createSound((const char*)sres, flags, NULL, &sound);
	if(errorCheck(result)) return;
	sound->setVariations(freqVar, 0, 0);
	errorCheck(result);

	if(sres)
		delete sres;
	mSounds.insert(std::make_pair(file, sound));
}


// Play a sound (with specified pan)
FMOD::Channel* FMODExSoundSystem::playSound(const String &file, Real pan) {
	if(mSoundDisabled) return NULL;
	Real vol;
	mSoundChannels->getVolume(&vol);
	if(vol <= 0) return NULL;

	FMOD::Channel *channel;
	FMOD_RESULT result = mSystem->playSound(FMOD_CHANNEL_FREE, mSounds[file], true, &channel);
	if(errorCheck(result)) return NULL;
	channel->setChannelGroup(mSoundChannels);
	channel->setPan(pan);
	channel->setPaused(false);
	return channel;
}


// Play a looped sound (returns the channel so the called can stop the sound)
FMOD::Channel * FMODExSoundSystem::playLoopedSound(const String &file) {
	if(mSoundDisabled) return NULL;
	Real vol;
	mSoundChannels->getVolume(&vol);
	if(vol <= 0) return NULL;

	FMOD::Channel *channel;
	FMOD_RESULT result = mSystem->playSound(FMOD_CHANNEL_FREE, mSounds[file], true, &channel);
	if(errorCheck(result)) return NULL;
	channel->setChannelGroup(mSoundChannels);
	channel->setPaused(false);
	return channel;
}


// Play some music
void FMODExSoundSystem::playMusic(const std::string& file) {
	if(mSoundDisabled) return;
	if(String(file).empty()) return;

	LogManager::getSingleton().logMessage("Playing '" + String(file) + "'..");

	// Release the previously playing music
	if(mMusic && mMusicChannel) {
		mMusicChannel->stop();
		mMusic->release();
		mMusic = 0;
	}

	// Disable the custom file system
	FMOD_RESULT result = mSystem->setFileSystem(0, 0, 0, 0, 2048);
	errorCheck(result);

	// Load
	result = mSystem->createStream(file.c_str(), FMOD_HARDWARE | FMOD_LOOP_OFF | FMOD_2D, 0, &mMusic);
	if(errorCheck(result)) {
		markBadSong();
		playMusic(getNextSong());

		// Reset the file system
		result = mSystem->setFileSystem(fileOpen, fileClose, fileRead, fileSeek, 2048);
		errorCheck(result);
		return;
	}

	// Play
	result = mSystem->playSound(FMOD_CHANNEL_FREE, mMusic, false, &mMusicChannel);
	errorCheck(result);

//	String svol = GameApplication::mGameConfig->getSetting("music_volume", "audio");
	String svol = GameApplication::mGameConfig->GetValue("audio", "music_volume", "1.0");
	Real vol = StringConverter::parseReal(svol);
	mMusicChannel->setVolume(vol);
	result = mMusicChannel->setCallback(FMOD_CHANNEL_CALLBACKTYPE_END, endCallBack, 0);
	errorCheck(result);

	// Reset the file system
	result = mSystem->setFileSystem(fileOpen, fileClose, fileRead, fileSeek, 2048);
	errorCheck(result);
}


// Set the music volume
void FMODExSoundSystem::setMusicVolume(Real vol) {
	if(mMusic && mMusicChannel)
		mMusicChannel->setVolume(vol);
}


// Set the sound volume
void FMODExSoundSystem::setSoundVolume(Real vol) {
	if(mSoundChannels)
		mSoundChannels->setVolume(vol);
}


// FMODExSoundSystem constructor
FMODExSoundSystem::FMODExSoundSystem(SceneManager *mgr) {
	assert(mgr);
	mSystem = 0;
	mMusic = 0;
	mMusicChannel = 0;
// 	mSceneMgr = mgr;
// 	mPlayNextSong = false;

	// Create the System object and initialise
	FMOD_RESULT result = FMOD::System_Create(&mSystem);
	if(errorCheck(result))
		return;

	char ver[32] = "";
	unsigned int version = 0;
	mSystem->getVersion(&version);
	sprintf(ver, "%08x", version);
	LogManager::getSingleton().logMessage("Using FMOD Ex v. " + String(ver));

	String sounds = GameApplication::mGameConfig->GetValue("audio", "sounds", "on");
	if(sounds.compare("off") == 0) {
		mSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		mSoundDisabled = true;
	}
	else
		mSoundDisabled = false;

	// Make FMOD to use ALSA on Linux, if specified on the gamesettings.cfg
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	if(!mSoundDisabled) {
		String alsa = GameApplication::mGameConfig->GetValue("audio", "fmod_use_alsa", "on");
		if(alsa.compare("off") != 0) {
			result = mSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
			if(errorCheck(result))
				return;
		}
	}
#endif

	result = mSystem->init(128, FMOD_INIT_NORMAL, 0);
	if(errorCheck(result))
		return;

	// Set the file system
	result = mSystem->setFileSystem(fileOpen, fileClose, fileRead, fileSeek, 2048);
	errorCheck(result);

	// Set the volume
	String svol = GameApplication::mGameConfig->GetValue("audio", "volume", "1.0");
	Real vol = StringConverter::parseReal(svol);
	result = mSystem->createChannelGroup("Sounds", &mSoundChannels);
	errorCheck(result);
	mSoundChannels->setVolume(vol);

	LogManager::getSingleton().logMessage("FMODExSoundSystem created.");

	// Init the playlist
	initPlaylist();
}


// FMODExSoundSystem destructor
FMODExSoundSystem::~FMODExSoundSystem() {
	FMOD_RESULT result;

	while(!mSounds.empty()) {
		if((*mSounds.begin()).second) (*mSounds.begin()).second->release();
		mSounds.erase(mSounds.begin());
	}
	mSounds.clear();

	if(mMusic) {
		result = mMusic->release();
		errorCheck(result);
	}

	if(mSystem) {
		result = mSystem->release();
		errorCheck(result);
	}

	LogManager::getSingleton().logMessage("FMODExSoundSystem destroyed.");
}


// Update the sound system
void FMODExSoundSystem::update() {
	assert(mSystem);
	mSystem->update();
	if(mPlayNextSong) {
		playMusic(getNextSong());
		mPlayNextSong = false;
	}
}
FMODExSoundSystem *FMODExSoundSystem::getSingletonPtr() {
	return ms_Singleton;
}

FMODExSoundSystem &FMODExSoundSystem::getSingleton() {
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
