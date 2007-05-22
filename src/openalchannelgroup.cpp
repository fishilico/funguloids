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

#include "openalchannelgroup.h"


//----------------------------------------------------------------------------
ChannelGroup::ChannelGroup( const int number )
{
	mVolume = 1.0f;
	mId = number;
}


//----------------------------------------------------------------------------
ChannelGroup::~ChannelGroup()
{
	// clear channel vector (without deleting the Channel object, this is done in AudioInterface)
	mFreeChannels.erase( mFreeChannels.begin(), mFreeChannels.end() );
	mPlayingChannels.erase( mPlayingChannels.begin(), mPlayingChannels.end() );
	mExclusiveChannels.erase( mExclusiveChannels.begin(), mExclusiveChannels.end() );
}

//----------------------------------------------------------------------------
OpenALChannel* ChannelGroup::findFreeChannel()
{
	// do we have an unused Channel left
	if ( !mFreeChannels.empty() )
	{
		OpenALChannel* c = mFreeChannels.back();
		mFreeChannels.pop_back();
		return c;
	}
	else
	{
		// nothing in the free channels list, check playing channels for finished ones
		for ( ChannelList::iterator it = mPlayingChannels.begin(); it != mPlayingChannels.end(); )
		{
			if ( !(*it)->isPlaying() )
			{
				OpenALChannel* c = (*it);
				it = mPlayingChannels.erase( it );
				mFreeChannels.push_back( c );
			}
			else
			{
				++it;
			}
		}
		// now we check the free channel list again
		if ( !mFreeChannels.empty() )
		{
			OpenALChannel* c = mFreeChannels.back();
			mFreeChannels.pop_back();
			return c;
		}
		else
		{
			// Ok, let's take the first interruptable Channel from the playing list.
			// Since we append newly used Channel at the end and start searching from
			// the beginnning we can be sure that this one is the oldest.
			for ( ChannelList::iterator it = mPlayingChannels.begin(); it != mPlayingChannels.end(); )
			{
				if ( (*it)->isInterruptable() )
				{
					OpenALChannel* c = (*it);
					it = mPlayingChannels.erase( it );
					return c;
				}
				else
				{
					++it;
				}
			}
			// no free channel found, sorry...
			LogManager::getSingleton().logMessage("findFreeChannel(): cannot find a free OpenALChannel");
			return 0;
		}
	}
}

//----------------------------------------------------------------------------
OpenALChannel* ChannelGroup::play( ALuint buffer, const bool looped, const bool _interruptable, const float gain )
{
	// don't start looped and not interruptable sounds, do this with exclusive Channels only
	bool interruptable;
	if ( looped )
		interruptable = true;
	else
		interruptable = _interruptable;

	OpenALChannel* c = findFreeChannel();

	if ( c ) {
		mPlayingChannels.push_back( c );
		c->play( buffer, looped, interruptable, gain * mVolume );
		return c;
	}
	return 0;
}

//----------------------------------------------------------------------------
OpenALChannel* ChannelGroup::play3D( ALuint buffer, const float x, const float y, const float z, const bool looped, const bool _interruptable, const float gain )
{
	// don't start looped and not interruptable sounds, do this with exclusive Channels only
	bool interruptable;
	if ( looped )
		interruptable = true;
	else
		interruptable = _interruptable;

	OpenALChannel* c = findFreeChannel();

	if ( c ) {
		mPlayingChannels.push_back( c );
		c->play3D( buffer, x, y, z, looped, interruptable, gain * mVolume );
		return c;
	}
	return 0;
}

//----------------------------------------------------------------------------
void ChannelGroup::stop( )
{
	for ( ChannelList::iterator it = mPlayingChannels.begin(); it != mPlayingChannels.end(); )
	{
		(*it)->stop();
		++it;
	}
}


//----------------------------------------------------------------------------
OpenALChannel* ChannelGroup::requestExclusiveChannel( )
{
	OpenALChannel* c = findFreeChannel();
	if ( c ) {
		mExclusiveChannels.push_back( c );
		return c;
	}
	return 0;
}


//----------------------------------------------------------------------------
void ChannelGroup::releaseExclusiveChannel( OpenALChannel* c )
{
	ChannelList::iterator it = std::find( mExclusiveChannels.begin(), mExclusiveChannels.end(), c );

	if ( it != mExclusiveChannels.end() )
	{
		(*it)->stop();
		mExclusiveChannels.erase( it );
		mFreeChannels.push_back( c );
	}
}


//----------------------------------------------------------------------------
void ChannelGroup::setVolume( const float vol )
{
	mVolume = vol;
	for ( ChannelList::iterator it = mPlayingChannels.begin(); it != mPlayingChannels.end(); )
	{
		(*it)->setVolume( vol );
		++it;
	}
}

#endif
