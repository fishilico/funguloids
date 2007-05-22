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

#include "openalchannel.h"

void OpenALChannel::clear()
{
	mId = -1;
	mExclusive = false;
	mInterruptable = true;
	mPlaying = false;
	mLooping = false;
	mError = false;
	m3D = false;
	mPosition[0] = 0.0f;
	mPosition[1] = 0.0f;
	mPosition[2] = 0.0f;
	mVolume = 1.0f;
}

const ALfloat zeroALArray3[]={ 0.0, 0.0, 0.0 }; //!< A zero-vector for OpenAL
const ALfloat *zeroALVec3 = &zeroALArray3[0];

//----------------------------------------------------------------------------
OpenALChannel::OpenALChannel( const int id )
{
	clear();
	mId = id;

	ALenum err;
	alGetError(); // clear error record
	alGenSources( 1, &mSource );
	if( (err = alGetError()) != AL_NO_ERROR)
	{
		LogManager::getSingleton().logMessage("OpenALChannel(): ERROR creating source:" );
		mError = true;
	}

	alSourcef ( mSource, AL_PITCH,			1.0f		);
	alSourcef ( mSource, AL_GAIN,			1.0f		);
	alSourcefv( mSource, AL_VELOCITY,		(ALfloat*) zeroALVec3	);
	alSourcei ( mSource, AL_LOOPING,		AL_FALSE	);
	alSourcei ( mSource, AL_SOURCE_RELATIVE,AL_TRUE		); // set to relative mode with distance 0
	alSourcefv( mSource, AL_POSITION,		(ALfloat*)  zeroALVec3	); // (this is 2D mode)
	alSourcef ( mSource, AL_ROLLOFF_FACTOR,	0.0f		); // turn off distance calc

	LogManager::getSingleton().logMessage("OpenALChannel(): created new channel");
}


//----------------------------------------------------------------------------
OpenALChannel::~OpenALChannel()
{
	// stop the source if playing
	ALint state;
	alGetSourcei( mSource, AL_SOURCE_STATE, &state );
	if ( state == AL_PLAYING ) {
		alSourceStop( mSource );
	}
	// destroy the source
	alDeleteSources(1, &mSource );
	LogManager::getSingleton().logMessage("OpenALChannel(): destroyed channel");
}

//----------------------------------------------------------------------------
void OpenALChannel::setPosition( const float x, const float y, const float z )
{
	mPosition[0] = x;
	mPosition[1] = y;
	mPosition[2] = z;
	alSourcefv( mSource, AL_POSITION, mPosition );
	//felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::setPos(): channel %d @ (%f, %f, %f)", mId,x,y,z);

}

//----------------------------------------------------------------------------
void OpenALChannel::setVolume( const float vol )
{
	mVolume = vol;
	alSourcef( mSource, AL_GAIN, vol );
}

//----------------------------------------------------------------------------
void OpenALChannel::mute()
{
	alSourcef( mSource, AL_GAIN, 0.0f );
}

//----------------------------------------------------------------------------
void OpenALChannel::unmute()
{
	alSourcef( mSource, AL_GAIN, mVolume );
}


//----------------------------------------------------------------------------
void OpenALChannel::set2D( )
{
	alSourcef( mSource, AL_ROLLOFF_FACTOR, 0.0f );
	alSourcei( mSource, AL_SOURCE_RELATIVE, AL_TRUE
		);
	alSourcefv( mSource, AL_POSITION, (ALfloat*) zeroALVec3 );
	m3D = false;
	//felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::set2D(): channel %d set to 2D", mId);
}

//----------------------------------------------------------------------------
void OpenALChannel::set3D( )
{
	alSourcef( mSource, AL_ROLLOFF_FACTOR, 4.0f );
	alSourcef( mSource, AL_REFERENCE_DISTANCE, 500.0f );
	alSourcei( mSource, AL_SOURCE_RELATIVE, AL_FALSE );
//	alSourcefv( mSource, AL_POSITION, mPosition );
	m3D = true;
	//felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::set3D(): channel %d set to 3D", mId);
}


//----------------------------------------------------------------------------
void OpenALChannel::play( ALuint buffer, bool looped, bool interruptable, const float gain )
{
	// if playing or paused, stop it
	ALint state;
	alGetSourcei( mSource, AL_SOURCE_STATE, &state );
	if ( state == AL_PLAYING )  {
		alSourceStop( mSource );
	}
	// when in 3D mode, switch to 2D
	if ( m3D ) set2D();

	alSourcei( mSource, AL_BUFFER, buffer );
	alSourcef( mSource, AL_GAIN, gain );

	// enable looping if requested
	if ( looped )
	{
		alSourcei( mSource, AL_LOOPING, AL_TRUE );
		mLooping = true;
	}
	else
	{
		alSourcei( mSource, AL_LOOPING, AL_FALSE );
		mLooping = false;
	}
	alSourcePlay( mSource );
	mPlaying = true;
	mInterruptable = interruptable;

	//felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::play(): channel %d playing buffer %p", mId, buffer);
}

//----------------------------------------------------------------------------
void OpenALChannel::play3D( ALuint buffer, const float x, const float y, const float z, bool looped, bool interruptable, const float gain )
{
	// if playing or paused, stop it
	ALint state;
	alGetSourcei( mSource, AL_SOURCE_STATE, &state );
	if ( state == AL_PLAYING )  {
		alSourceStop( mSource );
	}

	// when in 3D mode, switch to 2D
	if ( !m3D ) set3D();

	setPosition( x, y, z );

	// enable looping if requested
	if ( looped )
	{
		alSourcei( mSource, AL_LOOPING, AL_TRUE );
		mLooping = true;
	}
	else
	{
		alSourcei( mSource, AL_LOOPING, AL_FALSE );
		mLooping = false;
	}
	alSourcei( mSource, AL_BUFFER, (ALuint)buffer );
	alSourcef( mSource, AL_GAIN, gain );

	alSourcePlay( mSource );
	mPlaying = true;
	mInterruptable = interruptable;

	////felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::play3D(): channel %d playing buffer %p @ ( %f, %f, %f )", mId, buffer, x,y,z);
}

//----------------------------------------------------------------------------
void OpenALChannel::stop()
{
	alSourceStop( mSource );
	alSourcei( mSource, AL_BUFFER, AL_NONE );
	mPlaying = false;
//	//felog( LOG_AUDIO, LL_DEBUG, "OpenALChannel::stop(): channel %d stopped", mId );
}


//----------------------------------------------------------------------------
bool OpenALChannel::isPlaying( )
{
	if ( mLooping )
		return true;

	if ( mPlaying )
	{
		ALint state;
		alGetSourcei( mSource, AL_SOURCE_STATE, &state );
		if ( state == AL_PLAYING )
			return true;
	}
	stop();

	return false;
}

#endif
