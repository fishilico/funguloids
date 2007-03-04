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

#ifndef OBJECT_SYSTEM_H
#define OBJECT_SYSTEM_H

#include <Ogre.h>
#include <OgreSingleton.h>
#include <list>
#include "movingobject.h"

using namespace std;
using namespace Ogre;

typedef map<String, MovingObject*> ObjectMapType;

// ObjectSystem
class ObjectSystem : public Singleton<ObjectSystem> {
private:
	ObjectMapType mObjectMap;				// Moving objects
	SceneManager *mSceneMgr;				// Scene manager

public:
	ObjectSystem(SceneManager *mgr);
	~ObjectSystem();

	static ObjectSystem &getSingleton();
	static ObjectSystem *getSingletonPtr();

	void addObject(const String &name, MovingObject *newObj);
	void destroyObject(const String &name);
	void destroyObject(const ObjectMapType::iterator i);
	MovingObject *findObject(const String &name);
	void destroyObjects();


	ObjectMapType::iterator getFirst() { return mObjectMap.begin(); }
	ObjectMapType::iterator getLast() { return mObjectMap.end(); }

	void findFreePosition(Vector3 &pos, const Vector3 &playerPos, Real radius, Real distance = 60.0f);
};

#endif
