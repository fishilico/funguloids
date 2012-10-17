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

#ifndef PLAYER_H
#define PLAYER_H

#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>
#include "movingobject.h"
#include "mushroom.h"
#include "soundsystem.h"

using namespace Ogre;

// Object type
const int OTYPE_PLAYER = OTYPE_MOVING_BASIC + 1;


class GameCamera;
class Base;
class InputHandler;

class Player : public MovingObject {
private:
	SceneNode *mFlareNode;			// Flare node
	RibbonTrail *mTrail;			// Beam trail
	GameCamera *mCamera;			// Camera
	Real mTrailWave;				// Trail waving counter
	bool mSpaceDown;				// Is the space key down?
	bool mEscDown;					// Is the ESC key down?
	long mScore;					// Player score
	long mNewScore;					// New score from the collected mushrooms
	long mNewScoreStep;				// How much to add per a mushroom?

	bool mIsDead;					// Is the player dead?
	bool mIsOnBase;					// Is the player on a base?
	Base *mOnBase;					// The base the player is on
	Base *mLastBase;				// The last base the player was on
	Real mDeathCount;				// Death (respawn) counter
	bool mChangeLevel;				// Can we change the level?
	Real mLevelChangeTimeout;		// Level change timeout
	Real mMinSpeed;					// Minimum speed
	int mCurrentLevel;				// Current level number
	bool mGoingToSpecialLevel;		// Is the player going to the special level?
	bool mInSpecialLevel;			// Is the player in the special level?
	Real mSpecialLevelEffectAlpha;	// Special level transition effect alpha
	Real mBallWormGlowAlpha;		// Ball worm glow effect alpha

	int mNextMushroomColor;			// Color of the next mushroom to be created
	int mMushroomCount[NUM_MUSHROOMS];		// Number of collected mushrooms (total)
	int mTotalMushrooms;					// Total number of mushrooms left (for the special level)
	int mMushroomLevelCount[NUM_MUSHROOMS];	// Number of collected mushrooms (in the current level)
	Real mMushroomUnloadCounter;	// Mushroom unloading counter
	bool mIsCounting;				// Counting mushrooms?
	int mBaseColor;					// Base color where we're at
	TextAreaOverlayElement *mMushroomCounters[NUM_MUSHROOMS];	// Mushroom counters
	TextAreaOverlayElement *mScoreCounter;			// Score counter
	TextAreaOverlayElement *mLevelCounter[2];		// Level counter
	TextAreaOverlayElement *mPlaytimeCounter;		// Play time counter
	TextureUnitState *mOverlayTexFade[2];			// Score overlay texture units for fading
	Real mScoreFade;								// Score counter fade
	Real mMushroomCounterFade[NUM_MUSHROOMS];		// Counter fade

	int mPlaytimeMin;				// Playtime minutes
	int mPlaytimeSec;				// Playtime seconds
	Real mPlaytime;					// Counts the play time seconds

	SoundChannel *mBaseSound;		// Base sound loop
	SoundChannel *mFlySound;		// Flying sound loop
	Real mConstantFlySoundCounter;	// Counter for the constant flying sound (flight1)
	Real mFlySoundCounter;			// Counter for the flying sounds

	bool mConfirmQuit;				// Is the quit confirmation on the screen?

public:
	Player(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, GameCamera *cam, bool managed=true);
	~Player();

	void move(Real delta, InputHandler *input);
	RibbonTrail *getTrail() const { return mTrail; }

	bool isConfirmQuit() const { return mConfirmQuit; }
	void quit();

	bool preCollision(MovingObject *other);
	void afterCollision(MovingObject *other);
	void die();
	void respawn();
	void wrapAround();
	void moveTo(const Vector3 &pos);
	void setLastBase(Base *base) { mLastBase = base; }

	void catchWhirler();
	void doBallWormGlow() { mBallWormGlowAlpha = 1.0f; }

	long getScore() const { return mScore; }

	bool isDead() const { return mIsDead; }
	bool isOnBase() const { return mIsOnBase; }
	bool isGoingToSpecialLevel() const { return mGoingToSpecialLevel; }
	GameCamera *getCamera() { return mCamera; }
};


// Wrap all the scenenodes around which are too far from the player
void wrapEverything(Player *player, SceneManager *sceneMgr);


#endif
