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

#ifndef SOUNDSTREAM_H
#define SOUNDSTREAM_H

#include <string>

// base class for audio streams in different format
// this is a "null" audio ouput and the default for StreamPlayer
class SoundStream {
public:
	SoundStream() {}
	virtual ~SoundStream() {}
	virtual bool play( const std::string& filename ) { return true; }
	virtual void stop() {}
	virtual bool update() { return true; } // false = end of stream
	virtual void setVolume(const float vol) {}
};

#endif

// THE END

