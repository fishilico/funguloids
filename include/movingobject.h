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

#ifndef MOVINGOBJECT_H
#define MOVINGOBJECT_H

#include <Ogre.h>
#include "object.h"
using namespace Ogre;

// Object type
const int OTYPE_MOVING_BASIC = 100 + 1;


class MovingObject : public Object {
protected:
	Vector3 mCurrentSpeed;		// Current speed
	Real mSpeed;				// Moving speed
	Real mAnimSpeed;			// Animation speed

	Real mMass;					// Object mass
	bool mIsStuck;				// Don't keep from moving at first, if the object is stuck in collision
	Real mStuckCount;			// Number of seconds stuck in collision
	Vector3 mSafePosition;		// Safe position (i.e. not colliding)

	bool mIsToBeDeleted;		// Is the object going to be deleted?

public:
	MovingObject(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed=true);

	virtual void move(Real delta);
	bool checkCollisions(const Vector3 &pos, Vector3 &vel, Real delta);
	virtual bool preCollision(MovingObject *other) { return true; }
	virtual void afterCollision(MovingObject *other) { }
	bool isStuck() const { return mIsStuck; }

	void setSpeed(Real sp) { mSpeed = sp; }
	Real getSpeed() const { return mSpeed; }
	void setCurrentSpeed(const Vector3 &sp) { mCurrentSpeed = sp; }
	Vector3 getCurrentSpeed() const { return mCurrentSpeed; }

	void setMass(Real m) { mMass = m; }
	Real getMass() const { return mMass; }

	void setAnimSpeed(Real sp) { mAnimSpeed = sp; }
	Real getAnimSpeed() const { return mAnimSpeed; }

	Vector3 getSafePosition() const { return mSafePosition; }
	virtual void updateAfterWrap();

	void setToBeDeleted() { mIsToBeDeleted = true; }
	bool isToBeDeleted() const { return mIsToBeDeleted; }
};

#endif
