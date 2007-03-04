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

#ifndef MUSHROOM_H
#define MUSHROOM_H

#include <Ogre.h>
#include "movingobject.h"
using namespace Ogre;

// Object type
const int OTYPE_MUSHROOM = OTYPE_MOVING_BASIC + 3;

// Mushroom colors
enum {
	MUSHROOM_RED = 0,
	MUSHROOM_GREEN,
	MUSHROOM_BLUE,
	NUM_MUSHROOMS
};


class Mushroom : public MovingObject {
private:
	SceneNode *mFlareNode;			// Flare node
	SceneNode *mRealNode;			// Where the mushroom really is
	Real mWaving;					// Mushroom wave counter
	Real mWaveSpeed;				// Waving speed
	Real mRotationSpeed;			// Rotation speed
	Vector3 mRotationAxis;			// Rotation axis
	int mColor;						// Mushroom color

public:
	Mushroom(int color, const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed=true);
	~Mushroom();

	static int mushroomCounter;

	Vector3 getRealPosition() const { return getPosition() + mRealNode->getPosition(); }
	bool preCollision(MovingObject *other);
	void move(Real delta);
	void updateAfterWrap();
	void pickUp();
	int getColor() const { return mColor; }
};



#endif
