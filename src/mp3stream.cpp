/****************************************************************************
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

#ifdef SOUND_STREAM_MP3
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include "Ogre.h"

#include "mp3stream.h"

using namespace std;
using namespace Ogre;

const int MP3_BUFFER_SIZE = 4096 * 8 * 8;
const int MP3_DATA_CHUNK = 4096;

//----------------------------------------------------------------------------
const char* MadErrorString( const mad_stream* code )
{
	return mad_stream_errorstr(code);
}

static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

ALenum MadGetFrameFormat( struct mad_header *Header ) {
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			return AL_FORMAT_MONO16;
			break;
		case MAD_MODE_DUAL_CHANNEL:
			return AL_FORMAT_STEREO16;
			break;
		case MAD_MODE_JOINT_STEREO:
			return AL_FORMAT_STEREO16;
			break;
		case MAD_MODE_STEREO:
			return AL_FORMAT_STEREO16;
			break;
		default:
			return AL_FORMAT_MONO16;
			break;
	}
}

//----------------------------------------------------------------------------
MP3Stream::MP3Stream()
{
	mInitialised = false;
	mFileLoaded = false;
	mMemoryFile.dataPtr = 0;
	mHasBeenPlaying = false;
	initialise();
}


//----------------------------------------------------------------------------
MP3Stream::~MP3Stream()
{
	shutdown();
}

//----------------------------------------------------------------------------
void MP3Stream::initialise()
{
	if ( ! mInitialised ) {
		// create some buffers
		alGenBuffers( MP3_NUM_BUFFERS, mBuffers);
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
void MP3Stream::shutdown()
{
	if ( mInitialised )
	{
		stop();
		alSourceStop(mSource);
		alSourcei( mSource, AL_BUFFER, AL_NONE );
		alDeleteSources(1, &mSource);
		checkALError( "deleting source" );
		alDeleteBuffers( MP3_NUM_BUFFERS, mBuffers );
		checkALError( "delete buffers" );

		mInitialised = false;
	}
}


//----------------------------------------------------------------------------
int MP3Stream::load( const std::string& filename )
{
	FILE* fh = fopen(filename.c_str(), "rb");
	if (fh != NULL)
	{
		mFilename = filename;
#ifndef _WIN32
		struct stat sbuf;
		if ( stat( filename.c_str(), &sbuf) != -1 )
		{
#else
		struct _stat sbuf;
		if (_fstat(fh->_file, &sbuf) != -1)
		{
#endif
			// allocate memory, padded with MAD_BUFFER_GUARD 0 bytes to decode the
			// last frame as well
			int size = sbuf.st_size;
			mMemoryFile.dataPtr = new unsigned char[size+MAD_BUFFER_GUARD];
			unsigned char* endptr = mMemoryFile.dataPtr;
			memset(endptr+size,0,MAD_BUFFER_GUARD);
			// read the file contents
			if ( mMemoryFile.dataPtr != NULL )
			{
				fread( mMemoryFile.dataPtr, 1, size, fh);
//				stringstream s; s << size << " bytes";
//				LogManager::getSingleton().logMessage("MP3Stream::load(): loaded " + filename + "( size: " + s.str() + ")" );
				mMemoryFile.dataRead = 0;
		        mMemoryFile.dataSize = size;

				mFileLoaded = true;
			}
			fclose(fh);
		}
		return 0;
	}
	else
	{
		// file error
		LogManager::getSingleton().logMessage("MP3Stream::load(): error opening " + filename );
		return 1;
	}
}

//----------------------------------------------------------------------------
void MP3Stream::unload( )
{
	if ( mFileLoaded )
	{
		delete mMemoryFile.dataPtr;
		mMemoryFile.dataPtr = 0;
	}
}

//----------------------------------------------------------------------------
void MP3Stream::rewindFile( )
{
	int result;
	if ( mFileLoaded )
	{
		// memory file
		mMemoryFile.dataRead = 0;
		mEOF = false;
	}
}

//----------------------------------------------------------------------------
void MP3Stream::logInfo( struct mad_header *Header )
{
	std::stringstream ss;

	/* Convert the layer number to it's printed representation. */
	ss << "mpeg 1 layer ";
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			ss << "I";
			break;
		case MAD_LAYER_II:
			ss << "II";
			break;
		case MAD_LAYER_III:
			ss << "III";
			break;
		default:
			ss << "(unexpected layer value)";
			break;
	}
 	ss << "\nChannel format: ";
	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			ss << "single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			ss << "dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			ss << "joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			ss << "normal LR stereo";
			break;
		default:
			ss << "(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
 	ss << "\nEmphasis: ";
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			ss << "no";
			break;
		case MAD_EMPHASIS_50_15_US:
			ss << "50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			ss << "CCITT J.17";
			break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			ss << "reserved(!)";
			break;
#endif
		default:
			ss << "(unexpected emphasis value)";
			break;
	}

	ss << "\nBitrate: " << Header->bitrate << "kb/s\nSample rate: " << Header->samplerate << " Hz\n";
	LogManager::getSingleton().logMessage("MP3Stream Info: " + ss.str() );
}

//----------------------------------------------------------------------------
bool MP3Stream::play( bool looped )
{
	// we can start playing only if we have an ogg source
	if ( mInitialised && mFileLoaded )
	{
		// is already playing?
		if( isPlaying() )
		{
			LogManager::getSingleton().logMessage("MP3Stream::play() called on playing stream");
			return true;
		}

		mLooped = looped;
		mFrameCount = 0;
		mEOF = false;
		mHeaderSkipped = false;
		mFormat = 0;
		mShiftPerSample = 0;
		mRemainingPCM = 0;

		// init MAD
		mad_stream_init(&mStream);
		mad_frame_init(&mFrame);
		mad_synth_init(&mSynth);
		mad_timer_reset(&mTimer);

		// fill all existing buffers
		for ( int i = 0; i < MP3_NUM_BUFFERS; i++ )
		{
			if( ! stream( mBuffers[i] ) )
			{
				LogManager::getSingleton().logMessage("MP3Stream::play() initial buffer() failed");
				return false;
			}
		}

		mHasBeenPlaying = true;

		// queue the filled buffers...
		alSourceQueueBuffers( mSource, MP3_NUM_BUFFERS, mBuffers );
		checkALError( "initial queue buffers" );
		// ...and start playing them
		alSourcePlay(mSource);
		checkALError( "play source" );
//		LogManager::getSingleton().logMessage("MP3Stream::play() playing...");

		return true;
	}
	else
	{
		LogManager::getSingleton().logMessage("MP3Stream::play() error: no file for playing, use open() or load()");
		return false;
	}
}


//----------------------------------------------------------------------------
bool MP3Stream::play(const std::string & filename)
{
	if ( load(filename) ) {
		LogManager::getSingleton().logMessage("MP3Stream::play() error: cannot play file " +  filename );
		return false;
	}
	return play();
}


//----------------------------------------------------------------------------
bool MP3Stream::isPlaying() const
{
	ALenum state;
	alGetSourcei(mSource, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

//----------------------------------------------------------------------------
bool MP3Stream::update()
{
	static int processed;
	bool moreData = true;

	alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
//	felog( LOG_AUDIO, LL_DEBUG, "MP3Stream::update(): %d buffers processed", processed );
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

		if ( !moreData ) return false;
	}

	return moreData;
}

//----------------------------------------------------------------------------
// fills a buffer with decoded data
bool MP3Stream::stream( ALuint buffer )
{
	static unsigned char data[MP3_BUFFER_SIZE];
	unsigned char* optr = data;
	int size = 0; // amount of decoded data buffered

	while(size < MP3_BUFFER_SIZE)
	{
		int available;
		// we might have some decoded pcm data left
		if ( mRemainingPCM ) {
			available = mSynth.pcm.length;
			int startpcm = available - mRemainingPCM;
/*			stringstream sr; sr << "MAD: remaining pcm: " << mRemainingPCM << " samples";
			LogManager::getSingleton().logMessage(sr.str());*/
			for ( int i = startpcm; i < available; i++ ) {
				// left channel
				signed short Sample = MadFixedToSshort(mSynth.pcm.samples[0][i]);
				*(optr++)=Sample&0xff;
				*(optr++)=Sample>>8;
				size += 2;

				// right channel if available
				if ( mFormat == AL_FORMAT_STEREO16 ) {
					Sample=MadFixedToSshort(mSynth.pcm.samples[1][i]);
					*(optr++)=Sample&0xff;
					*(optr++)=Sample>>8;
					size += 2;
				}
			}
			mRemainingPCM = 0;
		} else  {
			if ( mEOF ) {
				// we have definitely no more samples to fill a buffer with
				return false;
			}
		}

		bool nodata = true;
		while( !mEOF && nodata) {
			// try to feed MP3_DATA_CHUNK bytes of encoded mp3 data into mad's decoder buffer
			// until succeeded or the end of file has been reached
			unsigned int readbytes;
			unsigned char* startptr = mMemoryFile.dataPtr + mMemoryFile.dataRead;
			if ( mMemoryFile.dataRead + MP3_DATA_CHUNK > mMemoryFile.dataSize ) {
				// end of file
				readbytes = (mMemoryFile.dataSize - mMemoryFile.dataRead) + MAD_BUFFER_GUARD;
//				LogManager::getSingleton().logMessage("MAD: memfile EOF reached");
				mEOF = true;
			} else {
				readbytes = MP3_DATA_CHUNK;
			}
			mMemoryFile.dataRead += readbytes;

			// fill input stream buffer
	// 		stringstream s; s << "MAD: buffer " << readbytes << " bytes";
	// 		LogManager::getSingleton().logMessage(s.str());

			// no errro handling here
			mad_stream_buffer(&mStream,startptr,readbytes);

			// decode buffer
			mStream.error = static_cast<mad_error>(0);
			if( mad_frame_decode(&mFrame,&mStream ))
			{
				if(MAD_RECOVERABLE(mStream.error))
				{
					/* Do not print a message if the error is a loss of
					* synchronization and this loss is due to the end of
					* stream guard bytes. (See the comments marked {3}
					* supra for more informations about guard bytes.)
					*/
					if(mStream.error!=MAD_ERROR_LOSTSYNC)// || mStream.this_frame!=GuardPtr)
					{
						LogManager::getSingleton().logMessage("MAD: recoverable frame level error " + String(MadErrorString(&mStream)));
					} else {
						if ( mFrameCount != 0 )
							LogManager::getSingleton().logMessage("MAD: MAD_ERROR_LOSTSYNC");
					}
					// let's check whether we have left some undecoded data from the last call
					if( mStream.next_frame != 0 ) {
						int remain = ( mStream.bufend - mStream.next_frame);
						mMemoryFile.dataRead = mMemoryFile.dataRead - remain;
					}
					continue;
				}
				else
					if(mStream.error==MAD_ERROR_BUFLEN) {
						LogManager::getSingleton().logMessage("MAD: MAD_ERROR_BUFLEN");
						// rewind the memfile and ignore this error
						if( mStream.next_frame != 0 ) {
							int remain = ( mStream.bufend - mStream.next_frame);
							mMemoryFile.dataRead = mMemoryFile.dataRead - remain;
						}
						continue;
					} else {
						// critical error
						LogManager::getSingleton().logMessage("MAD: unrecoverable frame level error "+ String(MadErrorString(&mStream)));
						return false;
					}
			} else {
// 				LogManager::getSingleton().logMessage("MAD: DECODED!");
				nodata = false;
			}

		} // while

		// set format (mono/stereo) based on the info from the first frame
		if( mFrameCount==0 ) {
			mFormat = MadGetFrameFormat(&mFrame.header);
			if ( mFormat == AL_FORMAT_STEREO16 ) {
//				LogManager::getSingleton().logMessage("MAD: Stereo");
				mShiftPerSample = 2;
			} else {
//				LogManager::getSingleton().logMessage("MAD: Mono");
				mShiftPerSample = 1;
			}
			logInfo(&mFrame.header);
		}

		++mFrameCount;
//		mad_timer_add(&mTimer,mFrame.header.duration);

		// synth decoded buffer to pcm samples
		mad_synth_frame(&mSynth,&mFrame);

		// now copy the decoded frame data into the OpenAL buffer
		available = mSynth.pcm.length;

// 		stringstream sx; sx << "MAD: have " << available << " pcm samples";
// 		LogManager::getSingleton().logMessage(sx.str());

		int pcmlength;
		if ( (size + (available << mShiftPerSample)) > MP3_BUFFER_SIZE ) {
			pcmlength = (MP3_BUFFER_SIZE - size) >> mShiftPerSample;
			mRemainingPCM = available - pcmlength;
		} else {
			pcmlength = available;
			mRemainingPCM = 0;
		}

		for ( int i = 0; i < pcmlength; i++ ) {
			// left channel
			signed short Sample = MadFixedToSshort(mSynth.pcm.samples[0][i]);
			*(optr++)=Sample&0xff;
			*(optr++)=Sample>>8;
			size += 2;

			// right channel if available
			if ( mFormat == AL_FORMAT_STEREO16 ) {
				Sample=MadFixedToSshort(mSynth.pcm.samples[1][i]);
				*(optr++)=Sample&0xff;
				*(optr++)=Sample>>8;
				size += 2;
			}
		}

		// let's check whether we have left some undecoded data from the last call
		if( mStream.next_frame != 0 ) {
			int remain = ( mStream.bufend - mStream.next_frame);
			mMemoryFile.dataRead = mMemoryFile.dataRead - remain;
/*			stringstream sr; sr << "MAD: remain: " << remain << " bytes";
			LogManager::getSingleton().logMessage(sr.str());*/
		}

// 		stringstream sy; sy << "MAD: size = " << size << ", remaining pcm samples = " << mRemainingPCM << ", frame: " << mFrameCount;
// 		LogManager::getSingleton().logMessage(sy.str());

	}

	if(size == 0)
	{
//		LogManager::getSingleton().logMessage("MP3Stream::stream(): end of streaming data reached" );
		return false;
	}

// 	LogManager::getSingleton().logMessage("MAD: AL buffer data");

	alBufferData( buffer, mFormat, data, size, mFrame.header.samplerate);
	checkALError( "buffer data in stream()" );

	return true;
}

//----------------------------------------------------------------------------
inline void MP3Stream::checkALError( const char* desc ) const
{
	int error = alGetError();
	if(error != AL_NO_ERROR)
		LogManager::getSingleton().logMessage("MP3Stream error: " + String(desc) + String(errorString(error)) );
}

//----------------------------------------------------------------------------
const std::string  MP3Stream::errorString( ALenum code ) const
{
	switch(code)
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
void MP3Stream::stop( )
{
	alSourceStop( mSource );
	alSourcei( mSource, AL_BUFFER, AL_NONE );

	if ( mHasBeenPlaying ) {
		mad_synth_finish(&mSynth);
		mad_frame_finish(&mFrame);
		mad_stream_finish(&mStream);
	}
	mHasBeenPlaying = false;

//	LogManager::getSingleton().logMessage("MP3Stream::stop(): playback stopped" );
	if ( mFileLoaded ) {
		unload();
	}
}

//----------------------------------------------------------------------------
void MP3Stream::setVolume( float vol )
{
	alSourcef ( mSource, AL_GAIN, vol );
}

#endif

// THE END
