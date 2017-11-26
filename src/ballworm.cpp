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

#include "ballworm.h"
#include "player.h"
#include "gamecamera.h"
#include "effects.h"
#include "soundsystem.h"
#include "game.h"


// Ball worm constructor
BallWorm::BallWorm(Real orbitRad, Real rotSpeed, Real angleStep, Player *player, const String &name, SceneManager *sceneMgr, const Vector3 &pos, bool managed) : Asteroid(name, sceneMgr, "", "", pos, managed) {
	mType = OTYPE_BALLWORM;
	mRotationSpeed = rotSpeed;
	mCurrentSpeed.x = 0; //Math::RangeRandom(-10, 10);
	mCurrentSpeed.y = 0; //Math::RangeRandom(-10, 10);
	mCurrentSpeed.z = 0;
	mPlayer = player;

	mOrbitRadius = orbitRad;
	mRotationAxis = Vector3::UNIT_Z;

	// Create the balls
	for(int f=0; f<numBalls; f++) {
		mBallDestroyed[f] = false;
		mBalls[f] = mNode->createChildSceneNode(mName + "BallNode" + StringConverter::toString(f));
		mBalls[f]->translate(mOrbitRadius, 0, 0, SceneNode::TS_WORLD);

		Entity *ent = mSceneMgr->createEntity(mName + "BallEntity" + StringConverter::toString(f), "WormBall.mesh");
		ent->setCastShadows(false);
		mBalls[f]->attachObject(ent);
		if(f%2) {
			mBalls[f]->setScale(0.7f, 0.7f, 0.7f);
		}
		mNode->rotate(mRotationAxis, Radian(Degree(angleStep))); // 18.5f
	}
}


// Ball worm destructor
BallWorm::~BallWorm() {
	// Destroy the balls
	for(int f=0; f<numBalls; f++) {
		SceneNode *nod = mBalls[f];
		nod->removeAndDestroyAllChildren();
		nod->detachAllObjects();
		mSceneMgr->destroyEntity(mName + "BallEntity" + StringConverter::toString(f));
		mSceneMgr->destroySceneNode(nod->getName());
	}
}


// Move the ball worm
void BallWorm::move(Real delta) {
	// Rotate
	mNode->rotate(mRotationAxis, Radian(mRotationSpeed * delta));

	// Move
	mNode->translate(mCurrentSpeed * delta);

	// Check collisions versus the player
	if(mPlayer->isDead() || mPlayer->isOnBase() || mPlayer->isGoingToSpecialLevel() || gameApp->getState() != STATE_GAME) return;

	Vector3 ppos = mPlayer->getPosition();

	for(int f=0; f<numBalls; f++) {
		if(mBallDestroyed[f]) continue;
		Vector3 d = mBalls[f]->_getDerivedPosition() - ppos;
		d.z = 0;
		Real dist = d.squaredLength();
		if(dist < 9) {
			// Collision
			//mPlayer->die();
			mPlayer->doBallWormGlow();
			mBalls[f]->setVisible(false);
			mBallDestroyed[f] = true;
			SoundSystem::getSingleton().playSound("balls");

			// Create an effect
			try {
				Effect *effect = new Effect(EFFECT_MUSHROOM_CATCH, mBalls[f]->getName() + "Effect", mSceneMgr, "", mBalls[f]->_getDerivedPosition());
				effect->createParticles("WormBallEffect");
				effect->setDuration(1.5f);
			}
			catch(...) {
				// Ignore..
			}
			break;
		}
	}
}
