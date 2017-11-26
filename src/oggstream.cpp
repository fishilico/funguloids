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

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef SOUND_STREAM_VORBIS

#include <sys/stat.h>
#include <sstream>
#include "Ogre.h"

#include "oggstream.h"

using namespace std;
using namespace Ogre;

// prototypes, definition at the end of this file
int _vorbisSeek( void *memfile, ogg_int64_t offset, int seekTo );
int _vorbisClose( void *memfile );
long _vorbisTell( void *memfile );
size_t _vorbisRead( void *ptr, size_t byteSize, size_t sizeToRead, void *memfile );

//----------------------------------------------------------------------------
OggStream::OggStream()
{
	mInitialised = false;
	mFileOpened = false;
	mFileLoaded = false;
	mOggMemoryFile.dataPtr = 0;
	initialise();
}


//----------------------------------------------------------------------------
OggStream::~OggStream()
{
	shutdown();
}

//----------------------------------------------------------------------------
void OggStream::initialise()
{
	if ( ! mInitialised ) {
		// create some buffers
		alGenBuffers( OGG_NUM_BUFFERS, mBuffers);
		checkALError( "generating buffers" );
		alGenSources(1, &mSource);
		checkALError( "generating source" );

		alSourcef ( mSource, AL_PITCH,			1.0f		);
		alSourcef ( mSource, AL_GAIN,			1.0f		);
		alSource3f( mSource, AL_VELOCITY,		0.0, 0.0, 0.0);
		alSource3f( mSource, AL_POSITION,        0.0, 0.0, 0.0);
		alSource3f( mSource, AL_VELOCITY,        0.0, 0.0, 0.0);
		alSource3f( mSource, AL_DIRECTION,       0.0, 0.0, 0.0);
		alSourcef ( mSource, AL_ROLLOFF_FACTOR,  0.0          );
		alSourcei ( mSource, AL_SOURCE_RELATIVE, AL_TRUE      );

		mInitialised = true;
	}
}

//----------------------------------------------------------------------------
void OggStream::shutdown()
{
	if ( mInitialised )
	{
		stop();
		alSourceStop(mSource);
		alSourcei( mSource, AL_BUFFER, AL_NONE );
		alDeleteSources(1, &mSource);
		checkALError( "deleting source" );
		alDeleteBuffers( OGG_NUM_BUFFERS, mBuffers );
		checkALError( "delete buffers" );
		mInitialised = false;
	}
}


//----------------------------------------------------------------------------
int OggStream::load( const std::string& filename )
{
	FILE* fh = fopen(filename.c_str(), "rb");
	if (fh != NULL)
	{
		mOggFilename = filename;
#ifndef _WIN32
		struct stat sbuf;
		if ( stat( filename.c_str(), &sbuf) != -1 )
		{
#else
		struct _stat sbuf;
		if (_fstat(fh->_file, &sbuf) != -1)
		{
#endif
			// allocate memory
			int size = sbuf.st_size;
			mOggMemoryFile.dataPtr = new char[size];
			// read the file contents
			if ( mOggMemoryFile.dataPtr != NULL )
			{
				fread( mOggMemoryFile.dataPtr, 1, size, fh);
//				LogManager::getSingleton().logMessage("OggStream::load(): loaded " + filename );
				mOggMemoryFile.dataRead = 0;
		        mOggMemoryFile.dataSize = size;

				// set callback functions
				mVorbisCallbacks.read_func = _vorbisRead;
				mVorbisCallbacks.close_func = _vorbisClose;
				mVorbisCallbacks.seek_func = _vorbisSeek;
				mVorbisCallbacks.tell_func = _vorbisTell;

				// open the stream
				int result;
				if ( (result = ov_open_callbacks( &mOggMemoryFile, &mOggStream, NULL, 0, mVorbisCallbacks)) != 0)
				{
					unload();
					LogManager::getSingleton().logMessage("OggStream::load(): error: ov_open_callbacks : " +String(errorString(result)) );
					return 1;
				}
				// get info on the stream
				mVorbisInfo = ov_info( &mOggStream, -1 );
				mVorbisComment = ov_comment( &mOggStream, -1 );

				// set format
				if( mVorbisInfo->channels == 1 )
					mFormat = AL_FORMAT_MONO16;
				else
					mFormat = AL_FORMAT_STEREO16;

//				LogManager::getSingleton().logMessage("OggStream::load(): file loaded");
				logInfo();
				mFileLoaded = true;
			}
			fclose(fh);
		}
		return 0;
	}
	else
	{
		// file error
		LogManager::getSingleton().logMessage("OggStream::load(): error opening " + filename );
		return 1;
	}
}

//----------------------------------------------------------------------------
void OggStream::unload( )
{
	if ( mFileLoaded )
	{
		ov_clear( &mOggStream );
		delete mOggMemoryFile.dataPtr;
		mOggMemoryFile.dataPtr = 0;
	}
}

//----------------------------------------------------------------------------
int OggStream::open( const std::string& filename )
{
	if(!(mOggFile = fopen(filename.c_str(), "rb")))
	{
		LogManager::getSingleton().logMessage("OggStream::open(): error: ould not open ogg file " + filename );
		return 1;
	}
	mOggFilename = filename;

	int result = ov_open(mOggFile, &mOggStream, NULL, 0);
	if( result < 0 )
	{
		fclose(mOggFile);
		LogManager::getSingleton().logMessage("OggStream::open(): error: ov_open : " +String(errorString(result)) );
		return 1;
	}
	// get info on the stream
	mVorbisInfo = ov_info( &mOggStream, -1 );
	mVorbisComment = ov_comment( &mOggStream, -1 );

	// set format
	if( mVorbisInfo->channels == 1 )
		mFormat = AL_FORMAT_MONO16;
	else
		mFormat = AL_FORMAT_STEREO16;

//	LogManager::getSingleton().logMessage("OggStream::open(): file opened");
	logInfo();
	mFileOpened = true;
	return 0;
}


//----------------------------------------------------------------------------
void OggStream::close( )
{
	if ( mFileOpened )
	{
		stop();
		ov_clear( &mOggStream );
		fclose( mOggFile );
		mOggFile = 0;
		mFileOpened = false;
	}
}

//----------------------------------------------------------------------------
void OggStream::rewindOggFile( )
{
	int result;
	if ( mFileLoaded )
	{
		// memory file
		mOggMemoryFile.dataRead = 0;
		ov_clear( &mOggStream );
		if ( (result = ov_open_callbacks( &mOggMemoryFile, &mOggStream, NULL, 0, mVorbisCallbacks)) != 0)
		{
			unload();
			LogManager::getSingleton().logMessage("OggStream::rewind(): error: ov_open_callbacks : " + String(errorString(result)) );
		}
		mVorbisInfo = ov_info( &mOggStream, -1 );
		mVorbisComment = ov_comment( &mOggStream, -1 );
	}
	else
	{
		// disk file
		if ( mFileOpened )
		{
			ov_clear( &mOggStream );
			fclose(mOggFile);
			mOggFile = fopen( mOggFilename.c_str(), "rb");
			if( (result = ov_open( mOggFile, &mOggStream, NULL, 0 )) < 0 )
			{
				close();
				LogManager::getSingleton().logMessage("OggStream::rewind(): error: ov_open : " + String( errorString(result ) ) );
				return;
			}
		}
	}


}

//----------------------------------------------------------------------------
void OggStream::logInfo()
{
	std::stringstream ss;
	ss	<< "version         " << mVorbisInfo->version         << "\n"
		<< "channels        " << mVorbisInfo->channels        << "\n"
		<< "rate (hz)       " << mVorbisInfo->rate            << "\n"
		<< "bitrate upper   " << mVorbisInfo->bitrate_upper   << "\n"
		<< "bitrate nominal " << mVorbisInfo->bitrate_nominal << "\n"
		<< "bitrate lower   " << mVorbisInfo->bitrate_lower   << "\n"
		<< "bitrate window  " << mVorbisInfo->bitrate_window  << "\n"
		<< "\n"
		<< "vendor " << mVorbisComment->vendor << "\n";

	for(int i = 0; i < mVorbisComment->comments; i++)
		ss << "   " << mVorbisComment->user_comments[i] << "\n";

	LogManager::getSingleton().logMessage("OggStream::info\n" + ss.str() );
}

//----------------------------------------------------------------------------
bool OggStream::play( bool looped )
{
	// we can start playing only if we have an ogg source
	if ( mInitialised && ( mFileOpened || mFileLoaded ) )
	{
		mLooped = looped;

		// is already playing?
		if( isPlaying() )
		{
			LogManager::getSingleton().logMessage("OggStream::play() called on playing stream");
			return true;
		}

		// fill all existing buffers
		for ( int i = 0; i < OGG_NUM_BUFFERS; i++ )
		{
			if( ! stream( mBuffers[i] ) )
			{
//				felog( LOG_AUDIO, LL_DEBUG, "OggStream::play() error stream()ing buffer %d", i);
				return false;
			}
		}

		// queue the filled buffers...
		alSourceQueueBuffers( mSource, OGG_NUM_BUFFERS, mBuffers );
		checkALError( "initial queue buffers" );
		// ...and start playing them
		alSourcePlay(mSource);
		checkALError( "play source" );
//		LogManager::getSingleton().logMessage("OggStream::play() playing...");

		return true;
	}
	else
	{
		LogManager::getSingleton().logMessage("OggStream::play() error: no file for playing, use open() or load()");
		return false;
	}
}


//----------------------------------------------------------------------------
bool OggStream::play(const std::string & filename)
{
	if ( load(filename) ) {
		LogManager::getSingleton().logMessage("OggStream::play() error: cannot play file " +  filename );
		return false;
	}
	return play();
}


//----------------------------------------------------------------------------
bool OggStream::isPlaying() const
{
	ALenum state;
	alGetSourcei(mSource, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

//----------------------------------------------------------------------------
bool OggStream::update()
{
	static int processed;
	bool moreData = true;

	alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
//	felog( LOG_AUDIO, LL_DEBUG, "OggStream::update(): %d buffers processed", processed );
	checkALError("get processed buffers");

	while( processed > 0 )
	{
		processed--;

		ALuint buffer;
		alSourceUnqueueBuffers(mSource, 1, &buffer);
		checkALError( "unqueue buffer during update" );

		moreData = stream(buffer);

		alSourceQueueBuffers(mSource, 1, &buffer);
		checkALError("queue buffer during update");
	}

	return moreData;
}

//----------------------------------------------------------------------------
bool OggStream::stream( ALuint buffer )
{
	static char data[OGG_BUFFER_SIZE];
	int  size = 0;
	int  section;
	int  result;

	while(size < OGG_BUFFER_SIZE)
	{
		result = ov_read( &mOggStream, data + size, OGG_BUFFER_SIZE - size, 0, 2, 1, &section );

		if(result > 0)
		{
			size += result;
//			felog( LOG_AUDIO, LL_DEBUG, "OggStream::stream(): size %d += result %d", size, result );
		}
		else
		{
			if(result < 0)
			{
				// error reading from the buffer
				LogManager::getSingleton().logMessage("OggStream::stream(): error: " + String(errorString(result)) );
				return false;
			}
			else
			{
				// result == 0 => end of memory buffer/file reached
				if ( mLooped )
				{
					LogManager::getSingleton().logMessage("OggStream::stream(): end reached, looping"); // (size=%d)", size);
					rewindOggFile();
				}
				else
				{
//					size = 0; // end condition
//					felog( LOG_AUDIO, LL_DEBUG, "OggStream::stream(): end reached, no loop (size=%d)", size);
					break;
				}
			}
		}
	}

	if(size == 0)
	{
//		felog( LOG_AUDIO, LL_DEBUG, "OggStream::stream(): end of song reached and size == 0" );
		return false;
	}

	alBufferData( buffer, mFormat, data, size, mVorbisInfo->rate );
	checkALError( "buffer data in stream()" );
// 	LogManager::getSingleton().logMessage("OggStream::buffer() BUFFERED");

	return true;
}

//----------------------------------------------------------------------------
inline void OggStream::checkALError( const char* desc ) const
{
	int error = alGetError();
	if(error != AL_NO_ERROR)
		LogManager::getSingleton().logMessage("OggStream error: " + String(desc) + String(errorString(error)) );
}

//----------------------------------------------------------------------------
const char* OggStream::errorString( int code ) const
{
    switch(code)
    {
        case OV_EREAD:
            return "Read from media.";
        case OV_ENOTVORBIS:
            return "Not Vorbis data.";
        case OV_EVERSION:
            return "Vorbis version mismatch.";
        case OV_EBADHEADER:
            return "Invalid Vorbis header.";
        case OV_EFAULT:
            return "Internal logic fault (bug or heap/stack corruption.";
        default:
            return "Unknown Ogg error.";
    }
}

//----------------------------------------------------------------------------
void OggStream::stop( )
{
	alSourceStop( mSource );
	alSourcei( mSource, AL_BUFFER, AL_NONE );
	LogManager::getSingleton().logMessage("OggStream::stop(): playback stopped" );
	if ( mFileLoaded ) {
		unload();
	}
}

//----------------------------------------------------------------------------
void OggStream::setVolume( float vol )
{
	alSourcef ( mSource, AL_GAIN, vol );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int _vorbisClose( void *memfile )
{
	// This file is called when we call ov_clear.  If we wanted, we could free our memory here, but
	// in this case, we will free the memory in the main body of the program, so dont do anything
	return 1;
}
//----------------------------------------------------------------------------
long _vorbisTell( void *memfile )
{
	// tell the vorbis lib how much data we have read so far
	return static_cast<OggMemoryFile*>(memfile)->dataRead;
}
//----------------------------------------------------------------------------
size_t _vorbisRead( void *ptr, size_t byteSize, size_t sizeToRead, void *memfile )
{
	size_t spaceToEOF;                     // How much more we can read till we hit the EOF marker
	size_t actualSizeToRead;       // How much data we are actually going to read from memory

	// Calculate how much we need to read.  This can be sizeToRead*byteSize or less depending on how near the EOF marker we are
	spaceToEOF = static_cast<size_t>(static_cast<OggMemoryFile*>(memfile)->dataSize - static_cast<OggMemoryFile*>(memfile)->dataRead);
	if ((sizeToRead*byteSize) < spaceToEOF)
			actualSizeToRead = (sizeToRead*byteSize);
	else
			actualSizeToRead = spaceToEOF;

	// A simple copy of the data from memory to the datastruct that the vorbis libs will use
	if (actualSizeToRead)
	{
			// Copy the data from the start of the file PLUS how much we have already read in
			memcpy(ptr, (char*)static_cast<OggMemoryFile*>(memfile)->dataPtr + static_cast<OggMemoryFile*>(memfile)->dataRead, actualSizeToRead);
			// Increase by how much we have read by
			static_cast<OggMemoryFile*>(memfile)->dataRead += (actualSizeToRead);
	}

	// Return how much we read (in the same way fread would)
	return actualSizeToRead;
}
//----------------------------------------------------------------------------
int _vorbisSeek(void *memfile, ogg_int64_t offset, int seekTo )
{
	ogg_int64_t spaceToEOF;             // How much more we can read till we hit the EOF marker
	ogg_int64_t actualOffset;   // How much we can actually offset it by
	OggMemoryFile* vorbisData;             // The data we passed in (for the typecast)

	// Get the data in the right format
	vorbisData = static_cast<OggMemoryFile*>(memfile);

	// goto where we wish to seek to
	switch ( seekTo )
	{
	case SEEK_SET:
		// to the start of the data
		if (vorbisData->dataSize >= offset)
				actualOffset = offset;
		else
				actualOffset = vorbisData->dataSize;
		vorbisData->dataRead = (int)actualOffset;
		break;
	case SEEK_CUR:
		// Seek from where we are
		// Make sure we dont go past the end
		spaceToEOF = (ogg_int64_t)(vorbisData->dataSize - vorbisData->dataRead);
		if (offset < spaceToEOF)
				actualOffset = (offset);
		else
				actualOffset = spaceToEOF;
		// Seek from our currrent location
		vorbisData->dataRead += actualOffset;
		break;
	case SEEK_END:
		// seek from the end of the file
		vorbisData->dataRead = vorbisData->dataSize+1;
		break;
	default:
		LogManager::getSingleton().logMessage("OggStream::_vorbisSeek(): unknown seek command" );
		break;
	};

	return 0;
}

#endif

// THE END
