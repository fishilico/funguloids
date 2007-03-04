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

#include "mushroom.h"
#include "effects.h"
#include "base.h"


// Mushroom counter
int Mushroom::mushroomCounter = 0;


// Mushroom constructor
Mushroom::Mushroom(int color, const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_MUSHROOM;
	mColor = color;
	mRotationSpeed = Math::RangeRandom(-3, 3);
	mCurrentSpeed.x = Math::RangeRandom(-10, 10);
	mCurrentSpeed.y = Math::RangeRandom(-10, 10);
	mCurrentSpeed.z = 0;

	mRotationAxis.x = Math::UnitRandom() * 1.5f;
	mRotationAxis.y = Math::UnitRandom() * 1.0f;
	mRotationAxis.z = Math::UnitRandom() * 0.2f;
	mRotationAxis.normalise();

	mRealNode = mNode->createChildSceneNode(name + "RealNode");
	mNode->detachObject(mEntity);
	mRealNode->attachObject(mEntity);
	mWaving = 0;
	mWaveSpeed = 1.0f + Math::UnitRandom() * 1.5f;
	if(Math::UnitRandom() < 0.5) mWaveSpeed = -mWaveSpeed;

	// Set the material
	mEntity->setMaterialName("MushroomMat" + StringConverter::toString(color + 1));
	setMass(1.5f);

	// Flare
	mFlareNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name + "FlareNode");
	mFlareNode->setPosition(getPosition());
	Entity *ent = mSceneMgr->createEntity(name + "FlareEntity", "Plane.mesh");
	ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_2);
	ent->setCastShadows(false);
	ent->setMaterialName("MushroomFlare" + StringConverter::toString(color + 1));
	mFlareNode->setScale(10, 10, 10);

	mFlareNode->attachObject(ent);
	Mushroom::mushroomCounter++;
}


Mushroom::~Mushroom() {
	mRealNode->detachAllObjects();
	mNode->removeAndDestroyAllChildren();
	mNode->detachAllObjects();
	mFlareNode->removeAndDestroyAllChildren();
	mFlareNode->detachAllObjects();
	mSceneMgr->destroyEntity(mName + "FlareEntity");
	mSceneMgr->destroySceneNode(mName + "FlareNode");
}


// Move the mushroom
void Mushroom::move(Real delta) {
	// Rotate
	mRealNode->rotate(mRotationAxis, Radian(mRotationSpeed * delta));

	// Wave
	mWaving += delta * mWaveSpeed;
	if(mWaving > 360.0f)
		mWaving -= 360.0f;

	// Check collisions
//	bool col = checkCollisions(getRealPosition(), mCurrentSpeed, delta);
//	if(!col) {
//		mSafePosition = getRealPosition();
		mNode->translate(mCurrentSpeed * delta);
//	}
//	else
//		setPosition(mSafePosition);

	// Compute the real position
	Vector3 dir = mCurrentSpeed.normalisedCopy();
	dir.z = dir.x; dir.x = -dir.y; dir.y = dir.z; dir.z = 0;
	Vector3 pos = dir*Math::Sin(mWaving) * 10;
	mRealNode->setPosition(pos);
//	if(!col)
//		mSafePosition = getRealPosition();

	mFlareNode->setPosition(getRealPosition());
}


// Update after wrapping around
void Mushroom::updateAfterWrap() {
	mFlareNode->setPosition(getRealPosition());
}


// Player picks the mushroom
void Mushroom::pickUp() {
	try {
		// Create the effect
		Effect *effect = new Effect(EFFECT_MUSHROOM_CATCH, mName + "Effect", mSceneMgr, "", getRealPosition());
		effect->createParticles("MushroomPickup" + StringConverter::toString(mColor+1));
		effect->setCurrentSpeed(mCurrentSpeed);

		// Create the sparks
		effect = new Effect(EFFECT_MUSHROOM_CATCH, mName + "EffectSparks", mSceneMgr, "", getRealPosition());
		effect->createParticles("MushroomSparks" + StringConverter::toString(mColor+1));
		effect->setDuration(1.0f);
		effect->setCurrentSpeed(mCurrentSpeed);

		ColourValue col;
		switch(mColor) {
			default:
			case MUSHROOM_RED: col = ColourValue(1,0,0); break;
			case MUSHROOM_GREEN: col = ColourValue(0,1,0); break;
			case MUSHROOM_BLUE: col = ColourValue(0,0,1); break;
		}
		effect->createLight(col);
	}
	catch(...) {
		// Ignore..
	}

}


// Handle the collision
bool Mushroom::preCollision(MovingObject *other) {
	mSafePosition = getRealPosition();
	setPosition(getRealPosition());
	mWaving = 0;
	if(other->getType() == OTYPE_BASE)
		mWaveSpeed = -mWaveSpeed;
	return true;
}
