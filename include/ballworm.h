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

#ifndef BALLWORM_H
#define BALLWORM_H

#include "asteroid.h"

// Object type
const int OTYPE_BALLWORM = OTYPE_ASTEROID + 100;

// Number of balls
const int numBalls = 5;

class Player;

class BallWorm : public Asteroid {
private:
	Real mOrbitRadius;					// Orbit radius
	SceneNode *mBalls[numBalls];		// Ball nodes
	bool mBallDestroyed[numBalls];		// Is the ball destroyed?
	Player *mPlayer;					// Player pointer

public:
	BallWorm(Real orbitRad, Real rotSpeed, Real angleStep, Player *player, const String &name, SceneManager *sceneMgr, const Vector3 &pos, bool managed=true);
	~BallWorm();

	void move(Real delta);
	
};

#endif
