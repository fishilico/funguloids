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

#ifndef OBJECT_H
#define OBJECT_H

#include <Ogre.h>
using namespace Ogre;

// Basic object type
const int OTYPE_BASIC = 0;


// Object
class Object {
protected:
	Entity *mEntity;				// Object entity
	SceneNode *mNode;				// Object node
	AnimationState *mAnimState;		// Object animation

	String mName;					// Object name
	SceneManager *mSceneMgr;		// SceneManager

	Real mRadius;					// Object radius

	int mType;						// Object type

public:
	Object(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed=true);
	virtual ~Object();
	int getType() const { return mType; }

	void setAnim(const String name) {
		mAnimState = mEntity->getAnimationState(name);
		mAnimState->setEnabled(true);
		mAnimState->setLoop(true);
	}

	void setScale(Real x, Real y, Real z) {
		mNode->setScale(x, y, z);
		setRadius(mEntity->getBoundingRadius());
	}

	void setRadius(Real rad) { mRadius = rad; }
	Real getRadius() const { return mRadius; }

	void updateAnim(Real time) {
		if(mAnimState)
			mAnimState->addTime(time);
	}

	Entity *getEntity() const {
		return mEntity;
	}

	SceneNode *getSceneNode() const {
		return mNode;
	}

	const String getName() const {
		return mName;
	}

	Vector3 getPosition() const {
		return mNode->getPosition();
	}

	void setPosition(const Vector3 &pos) {
		mNode->setPosition(pos);
	}

	void yaw(Radian angle) {
		mNode->yaw(angle);
	}

	void pitch(Radian angle) {
		mNode->pitch(angle);
	}

	void translate(Real x, Real y, Real z) {
		mNode->translate(x, y, z);
	}

	void translate(const Vector3 &trans) {
		mNode->translate(trans);
	}

	Vector3 getDir() const {
		return mNode->getOrientation() * Vector3::UNIT_Y;
	}

	void setDir(const Vector3 &dir) {
		mNode->setDirection(-dir, SceneNode::TS_WORLD, Vector3::NEGATIVE_UNIT_Y);
	}

	void setCastShadows(bool cast) {
		mEntity->setCastShadows(cast);
	}

	void setVisible(bool v) { mEntity->setVisible(v); }
	bool getVisible() const { return mEntity->isVisible(); }
};

#endif
