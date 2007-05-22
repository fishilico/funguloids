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

#ifndef OALCHANNEL_H
#define OALCHANNEL_H

#include "soundsystem.h"

#ifdef SOUND_SYSTEM_OPENAL

// OpenALChannel
class OpenALChannel
{
public:
	OpenALChannel( const int id = 0 );
	~OpenALChannel();

	void play( ALuint buffer, bool looped = false, bool interruptable = true, const float gain = 1.0 );
	void play3D( ALuint buffer, const float x, const float y, const float z, bool looped = false, bool interruptable = true, const float gain = 1.0 );
	void stop();

	void set3D();
	void set2D();

	void clear();

	void setPosition( const float x, const float y, const float z );
	void setVolume( const float vol );
	void mute();
	void unmute();

	inline bool isExclusive() { return mExclusive; }
	inline bool isInterruptable() { return mInterruptable; }
	inline bool isLooping() { return mLooping; }
	inline bool isError() { return mError; }

	inline void setExclusive( const bool state = true ) { mExclusive = state; }
	inline void setInterruptable( const bool state = true ) { mInterruptable = state; }
	inline void setLooping( const bool state = true ) { mLooping = state; }
	inline void setError( const bool state = true ) { mError = state; }

	bool isPlaying();

protected:
	ALuint mSource;
	int		mId;			//!< This Channel's id
	bool	mExclusive;		//!< If true, the channel has been claimed for exclusive use. Used for ChannelGroups.
	bool	mInterruptable; //!< If true, a playing sound can be stopped and replace by another one.
	bool	mPlaying;		//!< This is set to true when the channel starts playing. There is no automatic reset at the end of the buffer. Use
	bool	mLooping;		//!< Set to true if the channel is playing a looped sound.
	bool	mError;			//!< Is set to true if an error occurred while creating the underlying system object.
	bool	m3D;			//!< The play mode. Is true if the channel is in 3D mode.
	float	mPosition[3];	//!< The position of the sound source in 3D play mode.
	float	mVolume;		//!< Overall volume. Values range from [0..1]
};

#endif
#endif
