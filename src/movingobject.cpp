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

#include "movingobject.h"
#include "objectsystem.h"
#include "base.h"
#include "whirler.h"
#include "mushroom.h"
#include "effects.h"
#include "player.h"
#include "ballworm.h"
#include "blackhole.h"


// MovingObject constructor
MovingObject::MovingObject(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed) : Object(name, sceneMgr, mesh, pos, false) {
	mType = OTYPE_MOVING_BASIC;
	mSpeed = 100.0f;
	setAnimSpeed(1.5f);
	setMass(1.0f);
	mCurrentSpeed = Vector3::ZERO;
	mIsStuck = true;
	mStuckCount = 0;
	mSafePosition = pos;
	mIsToBeDeleted = false;

	// Add it to the Object system
	if(managed)
		ObjectSystem::getSingleton().addObject(name, this);
}


// Move the object
void MovingObject::move(Real delta) {
	// Model some friction
	mCurrentSpeed -= (mCurrentSpeed * delta);

	// Update the animation
	updateAnim(mSpeed * 0.2f * getAnimSpeed() * delta);

	// Move (and keep on the terrain)
	mNode->translate(mCurrentSpeed * delta);
}



// Do the collision detection
bool MovingObject::checkCollisions(const Vector3 &pos, Vector3 &vel, Real delta) {
	Vector3 npos = pos + vel*delta;
	bool collision = false;

	// Check against moving objects
	ObjectMapType::iterator j;
	ObjectSystem *objs = ObjectSystem::getSingletonPtr();
	for(j = objs->getFirst(); j != objs->getLast(); ++j) {
		MovingObject *other = (*j).second;
		if(other == this || other->isToBeDeleted()) continue;
		int otype = other->getType();
		if(otype == OTYPE_WHIRLER || otype == OTYPE_EFFECT || otype == OTYPE_BALLWORM || otype == OTYPE_BLACKHOLE) continue;
		if(getType() != OTYPE_PLAYER && otype == OTYPE_MUSHROOM) continue;

		// Check for collision
		Vector3 opos, ovel = other->getCurrentSpeed();
		if(otype != OTYPE_MUSHROOM)
			opos = other->getPosition();
		else
			opos = static_cast<Mushroom*>(other)->getRealPosition();


		Vector3 d = npos - opos;
		d.z = 0;
		Real dist = d.normalise();
		if(dist < (mRadius + other->getRadius()) * 0.75f) {
			// Collision
			mStuckCount += delta;

			// If we've been stuck for long, ignore the collision
			if(mStuckCount >= 0.75f) {
				return false;
			}

			// Call the pre-collision handler (returns false if we want to skip this collision)
			if(!preCollision(other)) continue;

			// Collision response
			if(other->getType() != OTYPE_BASE) {
				Real aA = vel.dotProduct(d);
				Real aB = ovel.dotProduct(d);

				Real P = (2.0f * (aA - aB)) / (mMass + other->getMass());

				Vector3 nvA = vel - P * other->getMass() * d;
				Vector3 nvB = ovel + P * mMass * d;
				vel = nvA;
				ovel = nvB;
				vel.z = ovel.z = 0;
			}
			else {
				vel = vel.reflect(d);
			}

			other->setCurrentSpeed(ovel);

			// Call the after-collision handler
			afterCollision(other);
			collision = true;
			//return true;
		}			
	}

	if(collision) return true;

	mStuckCount = 0;

	//mSafePosition = getPosition();//npos;
	return false;
}


void MovingObject::updateAfterWrap() {

}
