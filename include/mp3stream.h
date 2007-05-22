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

#ifndef FIRSTELEMENT_AUDIOMP3STREAM_H
#define FIRSTELEMENT_AUDIOMP3STREAM_H

#include <string>
#include <iostream>

#include <AL/al.h>
#include <mad.h>

#include "soundstream.h"

const int MP3_NUM_BUFFERS = 2;

//! \brief Struct that contains the pointer to our file in memory
struct MP3MemoryFile
{
	int		dataSize;	//!< Size of the data
	int		dataRead;	//!< How much data we have read so far
	unsigned char*	dataPtr;	//!< Pointer to the data in memoru
};


// a mp3 streaming class using libmad
class MP3Stream : public SoundStream {
public:
	MP3Stream();
	~MP3Stream();

	void initialise();	//!< Initialise the undelying OpenAL structures (source and buffers)
	void shutdown(); //!< Release the source and buffers.

//	int open( const std::string& filename ); //!< Open a file for file playback. Return something > 0 if error.
//	void close();	//!< Closes the file opened for file playback
//	bool isOpen() { return mFileOpened; }

	int load( const std::string& filename ); //!< Load an Ogg file into memory for playing. Return a value > 0 if error.
	void unload();	//!< Release the memory used by an loaded ogg file.
	bool isLoaded() { return mFileLoaded; }

	void logInfo( struct mad_header *Header );						//!< Log some ogg info.
	bool play( const std::string& filename );	//!< Play a file
	bool play( bool looped = false );	//!< Play the Ogg stream.
	void stop();						//!< Stop playback.
	bool isPlaying() const;				//!< Returns true if the source is actually playing.

	void setVolume( float vol );

	bool update();	//!< Refills all played buffers with new data.

protected:
	void rewindFile();
	bool stream( ALuint buffer );   //!< Fill all empty buffers. Returns false if the end of the file is reached.
	void checkALError( const char* desc ) const; //!< Check OpenAL error state and log errors. THIS HAS TO BE A CHAR* TO BE THREAD SAFE!!!
	const std::string errorString( ALenum code ) const; //!< Return a string holding the Ogg error message.

private:
	bool mInitialised;
	bool mFileLoaded;
	bool mLooped;					//!< Restart playing at the beginning of the file after playback has finished.

	unsigned int mFrameCount;
	bool mEOF;
	bool mHeaderSkipped;
	int mShiftPerSample; // for faster integer multiplication/division by 2 or 4
	int mRemainingPCM;
	bool mHasBeenPlaying;

	std::string mFilename;
	FILE* mFile;					//!< File handle for file playback.

	struct mad_stream	mStream;
	struct mad_frame	mFrame;
	struct mad_synth	mSynth;
	mad_timer_t			mTimer;

	MP3MemoryFile mMemoryFile;	//!< Our representation of the MP3 file in memory.

	ALuint mBuffers[MP3_NUM_BUFFERS];	//!< Buffers for playback.
	ALuint mSource;					//!< OpenAL audio source.
	ALenum mFormat;					//!< OpenAL sample data format.

};

#endif

// THE END
