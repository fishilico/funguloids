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

#include "blackhole.h"
#include "player.h"


// Black hole constructor
BlackHole::BlackHole(Real dist, Real effect, String name, Player *player, SceneManager *sceneMgr, String mesh, Vector3 pos, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_BLACKHOLE;
	mPlayer = player;
	mDistance = dist;
	mEffect = effect;

	mNode->scale(25, 25, 25);
	mEntity->setRenderQueueGroup(RENDER_QUEUE_3);
	mEntity->setMaterialName("BlackHole");
	setCastShadows(false);

	mPartsNode = mNode->createChildSceneNode(mName + "PartsNode");
	mPartsNode->setScale(0.05f, 0.05f, 0.05f);
	mParticles = mSceneMgr->createParticleSystem(mName + "Particles", "BlackHoleParts");
	mPartsNode->attachObject(mParticles);
}


// Update the black hole
void BlackHole::move(Real delta) {
	// Rotate
	mNode->roll(Radian(-0.5f * delta));

	// Direct the player away
	Vector3 d = getPosition() - mPlayer->getPosition();
	d.z = 0;
	if(d.squaredLength() <= mDistance*mDistance) {
		Real dist = d.normalise();
		Real effect = 1.0f - (dist / mDistance);
		Vector3 pdir = mPlayer->getDir();
		pdir -= effect * d * delta * mEffect;
		pdir.normalise();
		mPlayer->setDir(pdir);
	}
}


// Black hole destructor
BlackHole::~BlackHole() {
	if(mParticles) {
		mPartsNode->detachObject(mParticles);
		mSceneMgr->destroyParticleSystem(mParticles);
	}
}
