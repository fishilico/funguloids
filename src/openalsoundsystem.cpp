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

#include "soundsystem.h"

#ifdef SOUND_SYSTEM_OPENAL

#include "game.h"
#include "openalsoundsystem.h"
#include "playlist.h"
#include "mpakogre.h"

template<> OpenALSoundSystem* Singleton<OpenALSoundSystem>::msSingleton = 0;

OpenALSoundSystem* OpenALSoundSystem::getSingletonPtr() {
	return msSingleton;
}


OpenALSoundSystem& OpenALSoundSystem::getSingleton() {
	assert(msSingleton);
	return *msSingleton;
}


// OpenALSoundSystem constructor
OpenALSoundSystem::OpenALSoundSystem(SceneManager *mgr)
	: mSceneMgr(mgr),
	mPlayNextSong(false),
	mDevice(0),
	mContext(0),
	mIndexDefaultDevice(-1),
	mSoundChannels(0),
	mStreamPlayer(0)
{
	assert(mgr);

	mListenerPosition[0] = 0.0f;
	mListenerPosition[1] = 0.0f;
	mListenerPosition[2] = 0.0f;

	mListenerOrientation[0] = 0.0f;
	mListenerOrientation[1] = 0.0f;
	mListenerOrientation[2] = 1.0f;

	mListenerOrientation[3] = 0.0f; // up vector (positive y-axis is up)
	mListenerOrientation[4] = 1.0f;
	mListenerOrientation[5] = 0.0f;

	mListenerVelocity[0] = 0.0f;
	mListenerVelocity[1] = 0.0f;
	mListenerVelocity[2] = 0.0f;

//	int numDevs = getDevices();
	getDevices();

	String sounds = GameApplication::mGameConfig->GetValue("audio", "sounds", "on");
	if(sounds.compare("off") == 0) {
		mSoundDisabled = true;
	}
	else {
		if ( initialise() ) {
			mSoundDisabled = true;
			LogManager::getSingleton().logMessage("OpenALSoundSystem ERROR: cannot create SoundSystem.");
		} else {
			mSoundDisabled = false;
			LogManager::getSingleton().logMessage("OpenALSoundSystem created.");

		}
	}

	// Init the playlist
	initPlaylist();
}


// OpenALSoundSystem destructor
OpenALSoundSystem::~OpenALSoundSystem() {
	if ( ! mSoundDisabled )
	{
		shutdown();
	}
	LogManager::getSingleton().logMessage("OpenALSoundSystem destroyed.");
}

//----------------------------------------------------------------------------
std::string OpenALSoundSystem::lastALError( ALenum err )
{
	switch(err)
	{
		case AL_NO_ERROR:
			return std::string("AL_NO_ERROR");
			break;

		case AL_INVALID_NAME:
			return std::string("AL_INVALID_NAME");
			break;

		case AL_INVALID_ENUM:
			return std::string("AL_INVALID_ENUM");
			break;

		case AL_INVALID_VALUE:
			return std::string("AL_INVALID_VALUE");
			break;

		case AL_INVALID_OPERATION:
			return std::string("AL_INVALID_OPERATION");
			break;

		case AL_OUT_OF_MEMORY:
			return std::string("AL_OUT_OF_MEMORY");
			break;
		default:
			return std::string("AL_UNKNOWN_ERROR");
			break;
	}
}
//----------------------------------------------------------------------------
std::string OpenALSoundSystem::lastALUTError( ALenum err )
{
	std::string error(alutGetErrorString(err));
	return error;
}
//----------------------------------------------------------------------------
int OpenALSoundSystem::getDevices()
{
/*	std::string defaultDevice;
	// check whether we can query a list of available devices
	if ( alcIsExtensionPresent(0, "ALC_ENUMERATION_EXT") == AL_TRUE )
	{
		// get the name of the default device
		defaultDevice = (char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		LogManager::getSingleton().logMessage("OpenALSoundSystem: default device is " + defaultDevice );
		if (defaultDevice.length() > 0) {
			// fetch a string which contains zero separated substrings. Two zeros mark the end of the list.
			// this might change (which wouldn't be the worst thing to happen)
			char* deviceList = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			if (deviceList != 0) {
				for ( int i = 0; i < 12; i++ )
				{
					std::string device(deviceList); // create a string with the
					mDevices.push_back( device );
					LogManager::getSingleton().logMessage("OpenALSoundSystem(): found device " + device );
					if ( device == defaultDevice )
					{
						LogManager::getSingleton().logMessage("OpenALSoundSystem: (this is the default device)");
						mIndexDefaultDevice = i;
					}
					deviceList += device.length();
					// double 0 marks the end of the device list, a single 0 is the standart
					// end-of-string marker
					if (deviceList[0] == 0)
					{
						if (deviceList[1] == 0)
						{
							break;
						} else {
							deviceList += sizeof(char);
						}
					}
				}
			}
		}
	}
	else
	{
		LogManager::getSingleton().logMessage("OpenALSoundSystem(): no device enumeration available, trying the default device");
	}

	// sanity check the device list, if empty, add the default device specifier
	// this list might be empty() though
	if ( mDevices.empty() )
	{
		defaultDevice = (char *)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		if ( defaultDevice.length() > 0 )
		{
			LogManager::getSingleton().logMessage("OpenALSoundSystem(): added default device " + defaultDevice + " to list");
			mDevices.push_back( defaultDevice );
			mIndexDefaultDevice = 0;
		}
	}

	return mDevices.size();*/
	return 0;
}

//----------------------------------------------------------------------------
int OpenALSoundSystem::initialise( unsigned int devIndex )
{
	ALenum err;
/*	int index;
	// mDevices might be empty, so use the safe index -1 as a last resort. This catches bad indexes as well...
	if ( devIndex < mDevices.size() )
		index = static_cast<int>( devIndex );
	else
		index = -1;
*/
// 	if ( index == -1 ) {
// 		LogManager::getSingleton().logMessage("OpenALSoundSystem(): initialising OpenAL sound subsystem using the fallback method");
// 		// Default automatic initialisation via alutInit
// 		alutGetError();
// 		if ( ! alutInit(0,0) ) {
// 			LogManager::getSingleton().logMessage("OpenALSoundSystem: ERROR initialising ALUT: " + lastALUTError(alutGetError()) );
// 		} else {
// 			mSoundDisabled = false;
// 		}
// 		mContext = alcGetCurrentContext();
// 		mDevice = alcGetContextsDevice( mContext );
// 		char* cp = (char*) alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER );
// 		while ( strlen(cp) > 0 ) {
// 			LogManager::getSingleton().logMessage("OpenALSoundSystem(): default device specs are: " + String(cp) );
// 			cp += strlen(cp)+1;
// 		}
// 	} else {
// 		mDevice = alcOpenDevice( (mDevices[index].c_str()) );
// 		// create a context
// 		mContext = alcCreateContext( mDevice, 0 );
// 		// set active context
// 		alcMakeContextCurrent( mContext );
// 		alutGetError();
// 		if ( ! alutInitWithoutContext(0,0) ) {
// 			LogManager::getSingleton().logMessage("OpenALSoundSystem: ERROR initialising ALUT: " + lastALUTError(alutGetError()) );
// 		} else {
// 			mSoundDisabled = false;
// 			LogManager::getSingleton().logMessage("OpenALSoundSystem(): initialised OpenAL using device " + mDevices[index] );
// 		}
// 	}

	// check and initialise Ogg Vorbis support
// 	if (alcIsExtensionPresent( mDevice, "AL_EXT_vorbis") == AL_TRUE) {
// 		LogManager::getSingleton().logMessage("OpenALSoundSystem(): OGG Vorbis support available" );
// 	}


	mDevice = alcOpenDevice(0);
	if (mDevice == 0) {
		LogManager::getSingleton().logMessage("OpanAL: cannot create OpenAL device" );
		return 5;
	}

	int attributes[] = { 0 };
	mContext = alcCreateContext(mDevice, attributes);
	if ( (err = alcGetError(mDevice)) != ALC_NO_ERROR) {
		LogManager::getSingleton().logMessage("OpanAL: error creating context: " + lastALError(err) );
		return 1;
	}
//	check_alc_error("Couldn't create audio context: ");
	alcMakeContextCurrent(mContext);
//	check_alc_error("Couldn't select audio context: ");
	if ( (err = alcGetError(mDevice)) != ALC_NO_ERROR) {
		LogManager::getSingleton().logMessage("OpanAL: error making context current: " + lastALError(err) );
		return 2;
	}

	alutGetError();
	if ( ! alutInitWithoutContext(0,0) ) {
		LogManager::getSingleton().logMessage("OpenALSoundSystem: ERROR initialising ALUT: " + lastALUTError(alutGetError()) );
		return 3;
	} else {
		mSoundDisabled = false;
		LogManager::getSingleton().logMessage("OpenALSoundSystem(): initialised OpenAL" );
	}

	std::stringstream ss;
	ss << "OpenAL Vendor: " << alGetString(AL_VENDOR) << std::endl;
	ss << "OpenAL Version: " << alGetString(AL_VERSION) << std::endl;
	ss << "OpenAL Renderer: " << alGetString(AL_RENDERER) << std::endl;
	ss << "OpenAl Extensions: " << alGetString(AL_EXTENSIONS) << std::endl;
	LogManager::getSingleton().logMessage( ss.str() );

	// set listener position, orientation and velocity
	alListenerfv(AL_POSITION, mListenerPosition);
	alListenerfv(AL_VELOCITY, mListenerVelocity);
	alListenerfv(AL_ORIENTATION, mListenerOrientation);

	alDistanceModel( AL_INVERSE_DISTANCE );


	// add a ChannelGroup for sound
	int cgid = addChannelgroup( NUM_SOUND_CHANNELS );
	mSoundChannels = mChannelGroup[cgid];

	// Set the volume
	String svol = GameApplication::mGameConfig->GetValue("audio", "volume", "1.0");
	Real vol = StringConverter::parseReal(svol);
	mSoundChannels->setVolume(vol);

	// add a music player
	mStreamPlayer = new StreamPlayer();
	
	// Set the music volume
	svol = GameApplication::mGameConfig->GetValue("audio", "music_volume", "1.0");
	vol = StringConverter::parseReal(svol);
	mStreamPlayer->setVolume(vol);
	
	return 0;
}

//----------------------------------------------------------------------------
void OpenALSoundSystem::shutdown()
{
	if ( mStreamPlayer ) {
		mStreamPlayer->stop();
		delete mStreamPlayer;
		mStreamPlayer = 0;
	}

	if ( ! mSoundDisabled ) {
		stopAllChannels();
		alutExit();
		if(mContext) {
			alcDestroyContext(mContext);
			mContext = 0;
		}
		if(mDevice) {
			alcCloseDevice(mDevice);
			mDevice = 0;
		}
		mSoundDisabled = true;
	}
}

//----------------------------------------------------------------------------
int OpenALSoundSystem::addChannel()
{
	if ( ! mSoundDisabled )
	{
		unsigned int newIndex = mChannel.size();

		OpenALChannel* c = new OpenALChannel( newIndex );

		if ( c->isError() ) {
			delete c;
			return -1;
		}
		mChannel.push_back( c );

		return newIndex;
	}
	return -1;
}

//----------------------------------------------------------------------------
void OpenALSoundSystem::stopAllChannels()
{
	for ( ChannelVec::iterator it = mChannel.begin(); it != mChannel.end();  ) {
		(*it)->stop();
		++it;
	}
}

//----------------------------------------------------------------------------
int OpenALSoundSystem::addChannelgroup( unsigned int numChannels )
{
	unsigned int cgindex = mChannelGroup.size();

	// create new ChannelGroup
	ChannelGroup* cg = new ChannelGroup();
	// store a ChannelGroup pointer for access
	mChannelGroup.push_back( cg );

	LogManager::getSingleton().logMessage("AudioInterface(): added ChannelGroup " + Ogre::StringConverter::toString(cgindex) );

	// Add the requested number of channels
	for ( unsigned int i = 0; i < numChannels; i++ ) {
		int c = addChannel();
		cg->addChannel( mChannel[c] );
		LogManager::getSingleton().logMessage( "SoundSystem(): added Channel " +  Ogre::StringConverter::toString(c) + " to ChannelGroup");
	}
	// return the ChannelGroup's index
	return cgindex;
}

// Load a sound (with specified frequency variation)
void OpenALSoundSystem::loadSound(const String &fileName, Real freqVar, bool looped) {
	if(mSoundDisabled) return;

	String soundFile = fileName + ".wav";
	LogManager::getSingleton().logMessage("Loading " + soundFile + "..");

	SoundResource *soundResource = new SoundResource();
	SoundLocator *soundLocator = static_cast<SoundLocator*>(ResourceGroupManager::getSingletonPtr());
	soundResource->fileName = soundFile;
	soundResource->fileArchive = soundLocator->findSound(soundFile);
	if(!soundResource->fileArchive) {
		LogManager::getSingleton().logMessage("Could not find " + soundFile + "!");
		return;
	}

	// open
	soundResource->streamPtr = soundResource->fileArchive->open(soundResource->fileName);
	if(!soundResource->streamPtr) {
		LogManager::getSingleton().logMessage("Could not open " + soundFile + "!");
		return;
	}
	// read
	size_t fileSize = soundResource->streamPtr->size();
	unsigned char* buffer = new unsigned char[fileSize];
	size_t bytesRead = soundResource->streamPtr->read(buffer, fileSize);
	// close
	soundResource->streamPtr->close();

	// create a ALBuffer from the sound file in memory
	ALenum err;
	alutGetError();
	ALuint bufferid = alutCreateBufferFromFileImage( buffer, bytesRead );
	if ( (err = alutGetError()) != AL_NO_ERROR) {
		LogManager::getSingleton().logMessage("loadSound: failed to create buffer for <" + fileName + ">, error: " + lastALUTError(err) );
		return;
	}

	// store the buffer id associated with a name for later retrieval
	mSounds.insert(std::make_pair(fileName,bufferid ));

	LogManager::getSingleton().logMessage("loadSound: successfully loaded into sound buffer " + Ogre::StringConverter::toString(bufferid) );

	delete buffer;
}


// Play a sound (with specified pan)
OpenALChannel* OpenALSoundSystem::playSound(const String &file, Real pan) {
	if(mSoundDisabled) return 0;

	OpenALChannel* channel = mSoundChannels->play( mSounds[file] );
	return channel;
}


// Play a looped sound (returns the channel so the called can stop the sound)
OpenALChannel* OpenALSoundSystem::playLoopedSound(const String &file) {
	if(mSoundDisabled) return NULL;
	OpenALChannel* channel = mSoundChannels->play( mSounds[file], true );
	return channel;
}


//----------------------------------------------------------------------------
void OpenALSoundSystem::setListenerPosition( float x, float y, float z, float angle )
{
	mListenerPosition[0] = x;
	mListenerPosition[1] = y;
	mListenerPosition[2] = z;

	mListenerOrientation[0] = cos( angle );
	mListenerOrientation[2] = -sin( angle );

	alListenerfv(AL_POSITION, mListenerPosition);
	alListenerfv(AL_ORIENTATION, mListenerOrientation);
}

// Play some music
void OpenALSoundSystem::playMusic(const std::string& file) {
	if(mSoundDisabled) return;
	if(file.empty()) return;
	LogManager::getSingleton().logMessage("Going to play '" + file + "'..");
	assert(mStreamPlayer);
	
	// Set the volume
	String svol = GameApplication::mGameConfig->GetValue("audio", "music_volume", "1.0");
	Real vol = StringConverter::parseReal(svol);
	
	mStreamPlayer->play(file);
	mStreamPlayer->setVolume(vol);
}


// Set the music volume
void OpenALSoundSystem::setMusicVolume(Real vol) {
	assert(mStreamPlayer);
	mStreamPlayer->setVolume(vol);
}


// Set the sound volume
void OpenALSoundSystem::setSoundVolume(Real vol) {
	if(mSoundChannels)
		mSoundChannels->setVolume(vol);
}


// Update the sound system
void OpenALSoundSystem::update() {
	assert(mStreamPlayer);
	if ( ( ! mStreamPlayer->update() ) || mPlayNextSong ) {
		// end of stream reached
		playMusic(getNextSong());
	}
}


#endif
