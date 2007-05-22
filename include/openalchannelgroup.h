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


#ifndef OALCHANNELGRP_H
#define OALCHANNELGRP_H

#include "soundsystem.h"

#ifdef SOUND_SYSTEM_OPENAL

#include "openalchannel.h"

//! \brief ChannelGroup
class ChannelGroup
{
public:
	ChannelGroup( const int number = 0 );
	~ChannelGroup();

	// add another OpenALChannel to this group.
	void addChannel( OpenALChannel* channel ) { mFreeChannels.push_back( channel ); }

	inline int numChannels() { return (mFreeChannels.size()+mExclusiveChannels.size()+mPlayingChannels.size()); }

	OpenALChannel* play( ALuint buffer, const bool looped = false, const bool interruptable = true, const float gain = 1.0 );
	OpenALChannel* play3D( ALuint buffer, const float x, const float y, const float z, const bool looped = false, const bool interruptable = true, const float gain = 1.0 );
	void stop();

	void setVolume( const float vol );
	inline float getVolume() const { return mVolume; }

	OpenALChannel* requestExclusiveChannel();
	void releaseExclusiveChannel( OpenALChannel* channel );

protected:
	OpenALChannel* findFreeChannel();

	int mId;
	float mVolume;

	typedef std::list<OpenALChannel*> ChannelList;
	ChannelList mFreeChannels; // This vector contains all currently unused channels for this group.
	// This vector contains all channels for exclusive use by the application. They will not be returned
	// by findFreeChannel()
	ChannelList mExclusiveChannels;
	// This vector contains all currently playing channels in this group (even when playback of a non-looped
	// sample has finished, use Channel::isPlaying() to find out whether it is actually playing).
	ChannelList mPlayingChannels;
};

#endif
#endif
