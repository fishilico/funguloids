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

#include "base.h"


// Base constructor
Base::Base(int color, const String &name, const String &level, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_BASE;
	mColor = color;
	mIsOpen = false;
	mLevelFile = level;
	mParticles = NULL;

	// Set the material
	mEntity->getSubEntity(1)->setMaterialName("BaseMat2_" + StringConverter::toString(color + 1));
	setMass(1000.0f);
	setCurrentSpeed(Vector3::ZERO);

	setAnim("Open");
	mAnimState->setLoop(false);

	mParticles = mSceneMgr->createParticleSystem(mName + "Particles", "BaseOpen" + StringConverter::toString(mColor+1));
	mNode->attachObject(mParticles);
	//mNode->showBoundingBox(true);
}


// Base destructor
Base::~Base() {
	if(mParticles) {
		mNode->detachObject(mParticles);
		mSceneMgr->destroyParticleSystem(mParticles);
	}
}


// Move the base
void Base::move(Real delta) {
	// Rotate
	mNode->roll(Radian(0.2f * delta));

	// Update the animation
	if(mIsOpen)
		updateAnim(0.3f * getAnimSpeed() * delta);

	mCurrentSpeed = Vector3::ZERO;
}


// Open the base
void Base::open() {
	if(mIsOpen) return;

	mIsOpen = true;
	mAnimState->setTimePosition(0);
/*	mEntity->getMesh()->_setBoundingSphereRadius(1.5f * getRadius());
	AxisAlignedBox box = mEntity->getMesh()->getBounds();
	box.scale(Vector3(1.5f,1.5f,1.5f));
	mEntity->getMesh()->_setBounds(box);
	mNode->_updateBounds();
*/
}

