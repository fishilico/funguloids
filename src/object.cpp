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

#include "object.h"
#include "objectsystem.h"

// Object constructor
Object::Object(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed) {
	assert(sceneMgr);
	mType = OTYPE_BASIC;
	mName = name;
	mSceneMgr = sceneMgr;
	mAnimState = 0;
	mRadius = 1.0f;
	
	mEntity = NULL;
	if(!mesh.empty())
		mEntity = mSceneMgr->createEntity(mName + "Ent", mesh);
	mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mName, pos);

	if(!mesh.empty()) {
		mNode->attachObject(mEntity);
		mRadius = mEntity->getBoundingRadius();
	}

	mNode->setFixedYawAxis(true, Vector3::UNIT_Z);
}


// Destructor
Object::~Object() {
	// Destroy the node and entity
	mNode->removeAndDestroyAllChildren();
	mNode->detachAllObjects();
	if(mEntity)
		mSceneMgr->destroyEntity(mEntity);
	mSceneMgr->destroySceneNode(getName());
}

