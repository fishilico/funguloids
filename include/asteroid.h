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

#ifndef ASTEROID_H
#define ASTEROID_H

#include <Ogre.h>
#include "movingobject.h"
using namespace Ogre;

// Object type
const int OTYPE_ASTEROID = OTYPE_MOVING_BASIC + 2;


class Asteroid : public MovingObject {
protected:
	Real mRotationSpeed;			// Rotation speed
	Vector3 mRotationAxis;			// Rotation axis

public:
	Asteroid(const String &name, SceneManager *sceneMgr, const String &mesh, const String &material, const Vector3 &pos, bool managed=true);

	void move(Real delta);
};



#endif
