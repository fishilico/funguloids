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

#include "objectsystem.h"
#include "game.h"

template<> ObjectSystem* Singleton<ObjectSystem>::msSingleton = 0;


// ObjectSystem constructor
ObjectSystem::ObjectSystem(SceneManager *mgr) {
	assert(mgr);
	mSceneMgr = mgr;
	mObjectMap.clear();

	LogManager::getSingleton().logMessage("ObjectSystem created.");
}

// ObjectSystem destructor
ObjectSystem::~ObjectSystem() {
	destroyObjects();
	LogManager::getSingleton().logMessage("ObjectSystem destroyed.");
}


// Add a new object to the system
void ObjectSystem::addObject(const String &name, MovingObject *newObj) {
	//mObjectMap[name] = newObj;
	mObjectMap.insert(std::make_pair(name, newObj));
}



// Destroy an object from the system
void ObjectSystem::destroyObject(const String &name) {
	ObjectMapType::iterator i = mObjectMap.find(name);
	if(i != mObjectMap.end()) {
		delete (*i).second;
		(*i).second = NULL;
		mObjectMap.erase(i);
		return;
	}
}


// Destroy an object from the system by iterator, and return the next iterator
void ObjectSystem::destroyObject(const ObjectMapType::iterator i) {
	delete (*i).second;
	(*i).second = NULL;
	mObjectMap.erase(i);
}


// Find an object from the system
MovingObject *ObjectSystem::findObject(const String &name) {
	ObjectMapType::iterator i = mObjectMap.find(name);
	return (i != mObjectMap.end()) ? (*i).second : NULL;
}


// Destroy all the objects in system
void ObjectSystem::destroyObjects() {
	LogManager::getSingleton().logMessage("Destroying " + StringConverter::toString(mObjectMap.size()) + " objects..");

	// Delete the objects
	while( !mObjectMap.empty() ) {
		if((*mObjectMap.begin()).second) delete (*mObjectMap.begin()).second;
		mObjectMap.erase(mObjectMap.begin());
	}
	mObjectMap.clear();
	LogManager::getSingleton().logMessage("Objects destroyed.");
}



// Find a free random position which is not visible to the player
void ObjectSystem::findFreePosition(Vector3 &pos, const Vector3 &playerPos, Real radius, Real distance) {
	bool ok = false;
	int count = 0;

	while(!ok) {
		count++;
		if(count > 1000) break;

		// Get a random position
		pos.x = playerPos.x + Math::RangeRandom(-playfieldWidth, playfieldWidth);
		pos.y = playerPos.y + Math::RangeRandom(-playfieldHeight, playfieldHeight);
		pos.z = 0;

		// Check the distance to the player
		if((pos - playerPos).length() < distance) continue;
		//if(Math::Abs((pos.x+playfieldWidth) - (playerPos.x+playfieldWidth)) < distance && Math::Abs((pos.y+playfieldHeight) - (playerPos.y+playfieldHeight)) < distance) continue;


		// Check for collisions
		ok = true;
		ObjectMapType::iterator i;
		for(i = mObjectMap.begin(); i != mObjectMap.end(); ++i) {
			Vector3 d = pos - (*i).second->getPosition();
			d.z = 0;
			Real dist = d.squaredLength();
			Real rad = (*i).second->getRadius();
			if(dist < (radius+rad)*(radius+rad)) {
				// They collide
				ok = false;
				break;
			}
		}
	}

	// Found the position, yay! :)
}


ObjectSystem *ObjectSystem::getSingletonPtr() {
	return msSingleton;
}

ObjectSystem &ObjectSystem::getSingleton() {
	assert(msSingleton);
	return *msSingleton;
}

