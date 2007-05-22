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

#include "streamplayer.h"

#ifdef SOUND_SYSTEM_OPENAL


//-----------------------------------------------------------------------------
StreamPlayer::StreamPlayer()
{
#ifdef SOUND_STREAM_VORBIS
	mOggStream = new OggStream();
#endif
#ifdef SOUND_STREAM_MP3
	mMP3Stream = new MP3Stream();
#endif

	mNullStream = new SoundStream();
	mCurrentStream = mNullStream;
}

//-----------------------------------------------------------------------------
StreamPlayer::~StreamPlayer()
{
	mCurrentStream->stop();
#ifdef SOUND_STREAM_VORBIS
	delete mOggStream;
#endif
#ifdef SOUND_STREAM_MP3
	delete mMP3Stream;
#endif
	delete mNullStream;
}

//-----------------------------------------------------------------------------
enum StreamFileType {
	FT_OGG,
	FT_MP3,
	FT_UNKNOWN
};

void StreamPlayer::play(const std::string & filename)
{
	// TODO: check file type (simple look at the end of the name
	StreamFileType type;
	if ( filename.rfind(".ogg") != std::string::npos ) {
		type = FT_OGG;
	} else {
		if ( filename.rfind(".mp3") != std::string::npos ) {
			type = FT_MP3;
		} else {
			type = FT_UNKNOWN;
			// TODO: log error
			return;
		}
	}

	switch ( type ) {
	case FT_OGG:
#ifdef SOUND_STREAM_VORBIS
		mCurrentStream->stop();
		mCurrentStream = mOggStream;
		mCurrentStream->play(filename);
#endif
		break;
	case FT_MP3:
#ifdef SOUND_STREAM_MP3
		mCurrentStream->stop();
		mCurrentStream = mMP3Stream;
		mCurrentStream->play(filename);
#endif
		break;
	case FT_UNKNOWN:
		mCurrentStream->stop();
		mCurrentStream = mNullStream;
		mCurrentStream->play(filename);
		break;
	}
}

//-----------------------------------------------------------------------------
void StreamPlayer::stop()
{
	mCurrentStream->stop();
	mCurrentStream = mNullStream;
}

//-----------------------------------------------------------------------------
void StreamPlayer::setVolume(const float vol)
{
	mCurrentStream->setVolume(vol);
}

//-----------------------------------------------------------------------------
bool StreamPlayer::update()
{
	return mCurrentStream->update();
}

#endif

// THE END
