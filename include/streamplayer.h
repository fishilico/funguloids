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

#ifndef STREAMPLAYER_H
#define STREAMPLAYER_H

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef SOUND_SYSTEM_OPENAL

#ifdef SOUND_STREAM_VORBIS
	#include "oggstream.h"
#endif
#ifdef SOUND_STREAM_MP3
	#include "mp3stream.h"
#endif


#include <string>

// a stream player class that supports Ogg/Vorbis and MP3 stream decoding
class StreamPlayer {
public:
	StreamPlayer();
	~StreamPlayer();

	void play( const std::string& filename );
	void stop();

	void setVolume(const float vol);
	bool update();

protected:
#ifdef SOUND_STREAM_VORBIS
	OggStream* mOggStream;
#endif

#ifdef SOUND_STREAM_MP3
	MP3Stream* mMP3Stream;
#endif

	SoundStream* mNullStream; // default null output

	SoundStream* mCurrentStream; // active stream
};

#endif

#endif

// THE END
