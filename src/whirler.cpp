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

#include "whirler.h"
#include "objectsystem.h"
#include "player.h"
#include "effects.h"
#include "game.h"


// Whirler constructor
Whirler::Whirler(String name, Player *player, SceneManager *sceneMgr, String mesh, Vector3 pos, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_WHIRLER;
	mRotationSpeed = 3;
	mCurrentSpeed.x = Math::RangeRandom(-10, 10);
	mCurrentSpeed.y = Math::RangeRandom(-10, 10);
	mCurrentSpeed.z = 0;
	mPlayer = player;

	// Flare
	mFlareNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(name + "FlareNode");
	mFlareNode->setPosition(getPosition());
	Entity *ent = mSceneMgr->createEntity(name + "FlareEntity", "Plane.mesh");
	ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_2);
	ent->setCastShadows(false);
	ent->setMaterialName("WhirlerFlare");
	mFlareNode->setScale(10, 10, 10);
	mFlareNode->attachObject(ent);

	setSpeed(65.0f);
	setCastShadows(false);

	// Create the beam trail
    NameValuePairList pairList;
	pairList["numberOfChains"] = "1";
	pairList["maxElements"] = "12";
	mTrail = static_cast<RibbonTrail*>(mSceneMgr->createMovableObject(mName + "Trail", "RibbonTrail", &pairList));
    mTrail->setMaterialName("WhirlerBeamTrail");
	mTrail->setTrailLength(15);
	mTrail->setRenderQueueGroup(Ogre::RENDER_QUEUE_2);

	mSceneMgr->getRootSceneNode()->createChildSceneNode(mName + "TrailNode")->attachObject(mTrail);
	
	mTrail->setInitialColour(0, 1, 0.8f, 0.1f);
	mTrail->setColourChange(0, 4.5f, 5.5f, 4.5f, 0);
	mTrail->setInitialWidth(0, 2);
	mTrail->setWidthChange(0, 2.5f);
	mTrail->addNode(mNode);
}


// Move the whirler
void Whirler::move(Real delta) {
	// Rotate
	mNode->roll(Radian(-10 * delta));
	mNode->pitch(Radian(mRotationSpeed * delta));
	mFlareNode->roll(Radian(delta * 0.25f));

	Vector3 pos = getPosition();

	// Try to avoid all moving objects
	ObjectMapType::const_iterator i;
	ObjectSystem *ob = ObjectSystem::getSingletonPtr();
	for(i=ob->getFirst(); i != ob->getLast(); ++i) {
		if((*i).second == this || (*i).second->getType() == OTYPE_MUSHROOM) continue;

		Vector3 d = pos - (*i).second->getPosition();
		d.z = 0;
		if(d.squaredLength() > 60.0f*60.0f) continue;
		Real dist = d.normalise();
		Real effect = 1.0f - (dist / 60.0f);

		mCurrentSpeed += d * effect * mSpeed * 3.0f * delta;
	}

	// ..and player as well
	Vector3 d = pos - mPlayer->getPosition();
	d.z = 0;
	if(d.squaredLength() <= 60.0f*60.0f) {
		Real dist = d.normalise();
		// If distance is very small, player catches it
		if(dist < 3.0f) {
			pickUp();
			setToBeDeleted();
			mPlayer->catchWhirler();
			return;
		}
		Real effect = 1.0f - (dist / 60.0f);
		mCurrentSpeed += d * effect * mSpeed * 10.0f * delta;
	}

	// Cap the speed
	if(mCurrentSpeed.squaredLength() >= 55*55) {
		mCurrentSpeed = mCurrentSpeed.normalisedCopy() * 55.0f;
	}

	// Move
	mNode->translate(mCurrentSpeed * delta);
	mFlareNode->setPosition(mNode->getPosition());
}



// Update after wrapping around
void Whirler::updateAfterWrap() {
	Vector3 pos = getPosition();
	mFlareNode->setPosition(pos);

	// Fix the trail
	for(unsigned int f=0; f<mTrail->getMaxChainElements(); f++) {
		BillboardChain::Element bcElem = mTrail->getChainElement(0, f);

		Vector3 sPos = bcElem.position;

		Real dx = pos.x - sPos.x;
		Real dy = pos.y - sPos.y;

		if(dx > playfieldWidth)
			sPos.x += playfieldWidth*2;
		else if(dx < -playfieldWidth)
			sPos.x -= playfieldWidth*2;
		if(dy > playfieldHeight)
			sPos.y += playfieldHeight*2;
		else if(dy < -playfieldHeight)
			sPos.y -= playfieldHeight*2;

		bcElem.position = sPos;
		mTrail->updateChainElement(0, f, bcElem);
	}
}


// Pick the whirler up
void Whirler::pickUp() {
	try {
		// Create the effect
		Effect *effect = new Effect(EFFECT_WHIRLER_CATCH, mName + "Effect", mSceneMgr, "", getPosition());
		effect->createParticles("WhirlerPickup");
		effect->setCurrentSpeed(mCurrentSpeed * 0.5f);

		// Create the sparks
		effect = new Effect(EFFECT_WHIRLER_CATCH, mName + "EffectSparks", mSceneMgr, "", getPosition());
		effect->createParticles("WhirlerSparks");
		effect->setDuration(1.5f);
		effect->setCurrentSpeed(mCurrentSpeed * 0.5f);

		ColourValue col(1,0.75f,0);
		effect->createLight(col);
	}
	catch(...) {
		// Ignore..
	}
}


// Whirler destructor
Whirler::~Whirler() {
	mFlareNode->removeAndDestroyAllChildren();
	mFlareNode->detachAllObjects();
	mSceneMgr->destroyEntity(mName + "FlareEntity");
	mSceneMgr->destroySceneNode(mName + "FlareNode");
	mSceneMgr->getSceneNode(mName + "TrailNode")->detachAllObjects();
	mSceneMgr->destroySceneNode(mName + "TrailNode");
	mSceneMgr->destroyRibbonTrail(mTrail);
}

