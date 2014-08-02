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

#include "game.h"
#include "player.h"
#include "gamecamera.h"
#include "objectsystem.h"
#include "mushroom.h"
#include "asteroid.h"
#include "bground.h"
#include "effects.h"
#include "scriptsystem.h"
#include "base.h"
#include <OgreOverlayManager.h>


// Player constructor
Player::Player(const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, GameCamera *cam, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_PLAYER;
	setAnimSpeed(0.15f);
	setSpeed(40.0f);
	mMinSpeed = 40.0f;
	setMass(5.0f);
	setRadius(2.0f);
	mSpaceDown = true;
	mEscDown = false;
	mTrailWave = 0.0f;
	mIsDead = false;
	mIsOnBase = true;
	mOnBase = NULL;
	mLastBase = NULL;
	mDeathCount = 0;
	mScore = 0;
	mNewScore = 0;
	mNewScoreStep = 0;
	mCamera = cam;
	mCurrentLevel = 1;
	mPlaytime = 0;
	mPlaytimeMin = 0;
	mPlaytimeSec = 0;
	mLevelChangeTimeout = 0;
	mIsCounting = false;
	mGoingToSpecialLevel = false;
	mInSpecialLevel = false;
	mSpecialLevelEffectAlpha = 0;
	mBallWormGlowAlpha = 0;

	mBaseSound = NULL;
	mFlySound = NULL;
	mConstantFlySoundCounter = 26.0f;
	mFlySoundCounter = Math::RangeRandom(4,6);

	ScriptSystem::getSingleton().setScriptNumber("currentLevel", mCurrentLevel);

	int f;
	for(f=0; f<NUM_MUSHROOMS; f++) {
		mMushroomCount[f] = 0;
		mMushroomLevelCount[f] = 0;
		mMushroomCounterFade[f] = 0;
		mMushroomCounters[f] = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Mushrooms" + StringConverter::toString(f+1)));
	}
	
	mMushroomUnloadCounter = 0;
	mTotalMushrooms = 0;
	mBaseColor = 0;
	mScoreCounter = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("Score"));
	mLevelCounter[0] = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("LevelNumber"));
	mLevelCounter[1] = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("LevelText"));
	mPlaytimeCounter = static_cast<TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement("PlayTime"));
	mChangeLevel = false;
	mNextMushroomColor = rand() % MUSHROOM_BLUE;

	char str[16] = "";
	sprintf(str, "%02d", mCurrentLevel);
	mLevelCounter[0]->setCaption(String(str));

	sprintf(str, "%02d:%02d", mPlaytimeMin, mPlaytimeSec);
	mPlaytimeCounter->setCaption(String(str));

	// Create the beam trail
    NameValuePairList pairList;
	pairList["numberOfChains"] = "1";
	pairList["maxElements"] = "40";
	mTrail = static_cast<RibbonTrail*>(mSceneMgr->createMovableObject("PlayerTrail", "RibbonTrail", &pairList));
    mTrail->setMaterialName("PlayerBeamTrail");
	mTrail->setTrailLength(90);
	mTrail->setRenderQueueGroup(Ogre::RENDER_QUEUE_2);

	mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerTrailNode")->attachObject(mTrail);
	
	mTrail->setInitialColour(0, 0, 0.8f, 1);
	mTrail->setColourChange(0, 1.5f, 1.5f, 1.5f, 1.5f);
	mTrail->setInitialWidth(0, 3);
	mTrail->addNode(mNode);

	// Create player light
	Light *plight = mSceneMgr->createLight("PlayerLight");
	plight->setType(Light::LT_POINT);
	plight->setDiffuseColour(ColourValue(0.3f, 0.65f, 1)); // 0.3 0.3 1
	plight->setSpecularColour(ColourValue(1, 1, 1));
	plight->setCastShadows(true);
	mNode->attachObject(plight);


	// Create the flare
    ManualObject *manual = mSceneMgr->createManualObject("playerFlare");
	manual->begin("DropFlare", RenderOperation::OT_TRIANGLE_STRIP);

	Real s = 10;
    manual->position(-s, -s, 0); manual->colour(0.2f, 0.8f, 1, 1); manual->textureCoord(0, 0);
    manual->position(-s,  s, 0); manual->colour(0.2f, 0.8f, 1, 1); manual->textureCoord(0, 1);
    manual->position( s,  s, 0); manual->colour(0.2f, 0.8f, 1, 1); manual->textureCoord(1, 1);
	manual->position( s, -s, 0); manual->colour(0.2f, 0.8f, 1, 1); manual->textureCoord(1, 0);
    manual->index(2);
	manual->index(1);
    manual->index(3);
	manual->index(0);
    manual->end();
	//manual->setRenderQueueGroup(Ogre::RENDER_QUEUE_2);
	manual->setCastShadows(false);

	mFlareNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("PlayerFlareNode");
	mFlareNode->setPosition(getPosition());
	mFlareNode->attachObject(manual);
	mFlareNode->setScale(0.75f, 0.75f, 0.75f);

	// Score overlay fading stuff
	mScoreFade = 1;
	MaterialPtr mat = MaterialManager::getSingleton().getByName("BlockCenter");
	mOverlayTexFade[0] = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);
	mat = MaterialManager::getSingleton().getByName("BlockBorder");
	mOverlayTexFade[1] = mat->getTechnique(0)->getPass(0)->getTextureUnitState(0);

	mOverlayTexFade[0]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 1);
	mOverlayTexFade[1]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 1);

	// Make the scores visible
	mScoreFade = 1;
	ColourValue col = mScoreCounter->getColourTop();
	col.a = mScoreFade;
	mScoreCounter->setColourTop(col);
	col = mScoreCounter->getColourBottom();
	col.a = mScoreFade;
	mScoreCounter->setColourBottom(col);
	col = mPlaytimeCounter->getColourTop();
	col.a = mScoreFade;
	mPlaytimeCounter->setColourTop(col);

	// Make the level counter visible
	for(f=0; f<2; f++) {
		col = mLevelCounter[f]->getColourTop();
		col.a = mScoreFade;
		mLevelCounter[f]->setColourTop(col);
	}

	sprintf(str, "%06ld", mScore);
	mScoreCounter->setCaption(String(str));
	sprintf(str, "%02d", mMushroomCount[0]);
	for(f=0; f<NUM_MUSHROOMS; f++)
		mMushroomCounters[f]->setCaption(String(str));

	// Start the base sound
	mBaseSound = SoundSystem::getSingleton().playLoopedSound("base_hanging");
	mConfirmQuit = false;
}


// Player destructor
Player::~Player() {
	mSceneMgr->destroySceneNode("PlayerFlareNode");
	mSceneMgr->destroyManualObject("playerFlare");
	mSceneMgr->getSceneNode("PlayerTrailNode")->detachAllObjects();
	mSceneMgr->destroySceneNode("PlayerTrailNode");
	mSceneMgr->destroyRibbonTrail(mTrail);
	mSceneMgr->destroyLight("PlayerLight");
}


// Player has catched the whirler, transport to the special level
void Player::catchWhirler() {
	mGoingToSpecialLevel = true;
	mLevelChangeTimeout = 0.5f;
	mSpecialLevelEffectAlpha = 0;

	SoundSystem::getSingleton().playSound("whirler");
}


// Quit the game
void Player::quit() {
	gameApp->setEndGame();
	if(mFlySound) mFlySound->stop();
	if(mBaseSound) mBaseSound->stop();
	mSpecialLevelEffectAlpha = 0;
	mBallWormGlowAlpha = 0;
	OverlayManager::getSingleton().getByName("ConfirmQuit")->hide();
}


// Move the player
void Player::move(Real delta, InputHandler *input) {
	// Quit?
	if(input->isKeyDown(OIS::KC_ESCAPE)) {
		if(!mEscDown && !mConfirmQuit) {
			mConfirmQuit = true;
			mEscDown = true;
			OverlayManager::getSingleton().getByName("ConfirmQuit")->show();
			return;
		}
	}
	else
		mEscDown = false;

	if(mConfirmQuit) {
		if(input->isKeyDown(OIS::KC_ESCAPE) && !mEscDown) quit();
		else if(input->isKeyDown(OIS::KC_RETURN)) {
			mConfirmQuit = false;
			mSpaceDown = true;
			OverlayManager::getSingleton().getByName("ConfirmQuit")->hide();
		}
		return;
	}

	// Count the play time
	mPlaytime += delta;
	if(mPlaytime >= 1.0f) {
		mPlaytime -= 1.0f;
		mPlaytimeSec++;
		if(mPlaytimeSec >= 60) {
			mPlaytimeSec = 0;
			mPlaytimeMin++;
		}
		char str[16] = "";
		sprintf(str, "%02d:%02d", mPlaytimeMin, mPlaytimeSec);
		mPlaytimeCounter->setCaption(String(str));
	}

	// Do the special level transition effect
	if(mGoingToSpecialLevel) {
		mSpecialLevelEffectAlpha += delta*1.7f;
		if(mSpecialLevelEffectAlpha > 1.0f) mSpecialLevelEffectAlpha = 1.0f;
		gameApp->updateSpecialLevelTransitionEffect(mSpecialLevelEffectAlpha);
	}
	else if(mInSpecialLevel && mSpecialLevelEffectAlpha > 0) {
		mSpecialLevelEffectAlpha -= delta*0.5f;
		if(mSpecialLevelEffectAlpha < 0) mSpecialLevelEffectAlpha = 0;
		gameApp->updateSpecialLevelTransitionEffect(mSpecialLevelEffectAlpha);
	}


	// Do the ball worm glow effect
	if(mBallWormGlowAlpha > 0) {
		mBallWormGlowAlpha -= delta*0.5f;
		if(mBallWormGlowAlpha < 0) mBallWormGlowAlpha = 0;
		gameApp->updateBallWormGlowEffect(mBallWormGlowAlpha);
	}


	// If we're dead, end the game
	if(mIsDead) {
		mDeathCount -= delta;
		if(mDeathCount <= 3.0f && mTrail->isVisible()) mTrail->setVisible(false);
		if(mDeathCount <= 0) {
			gameApp->setEndGame();
			return;
		}
		//if(mDeathCount <= 0) respawn();
	}

	// Fade the scores
	if(mScoreFade > 0.0f && !mIsOnBase) {
		mScoreFade -= delta;
		if(mScoreFade < 0.0f) mScoreFade = 0.0f;
		ColourValue col = mScoreCounter->getColourTop();
		col.a = mScoreFade;
		mScoreCounter->setColourTop(col);
		col = mScoreCounter->getColourBottom();
		col.a = mScoreFade;
		mScoreCounter->setColourBottom(col);
		col = mPlaytimeCounter->getColourTop();
		col.a = mScoreFade;
		mPlaytimeCounter->setColourTop(col);

		for(int f=0; f<2; f++) {
			col = mLevelCounter[f]->getColourTop();
			col.a = mScoreFade;
			mLevelCounter[f]->setColourTop(col);
		}

		mOverlayTexFade[0]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);
		mOverlayTexFade[1]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);
	}

	// Fade the mushroom counters
	for(int f=0; f<NUM_MUSHROOMS; f++) {
		if(mMushroomCounterFade[f] > 0) mMushroomCounterFade[f] -= delta;
		if(mMushroomCounterFade[f] < 0) mMushroomCounterFade[f] = 0;
		ColourValue col = mMushroomCounters[f]->getColourTop();
		col.a = mMushroomCounterFade[f];
		mMushroomCounters[f]->setColourBottom(col);
		col.a = mScoreFade;
		mMushroomCounters[f]->setColourTop(col);
	}

	// If we're on the special level, we have to unload all the mushrooms
	if(mInSpecialLevel && mIsCounting) {
		if(mMushroomCount[mBaseColor] == 0) {
			// Unload the bases in sequence
			mMushroomUnloadCounter += 0.33f;
			switch(mBaseColor) {
				case BASE_BLUE:  mBaseColor = BASE_GREEN; break;
				case BASE_GREEN: mBaseColor = BASE_RED; break;
				case BASE_RED: mLevelChangeTimeout = 0.5f; mInSpecialLevel = false; break;
			}
		}
	}

	// Unload the mushrooms if we're on a base
	if(mIsOnBase && mTotalMushrooms > 0) {
		mMushroomUnloadCounter -= delta;
		if(mMushroomUnloadCounter <= 0) {
			mMushroomCount[mBaseColor]--;
			mTotalMushrooms--;
			if(mMushroomCount[mBaseColor] > 0)
				mMushroomUnloadCounter += 0.2f;

			// Update the score
			if(mTotalMushrooms > 0)
				mScore += mNewScoreStep;
			else
				mScore += mNewScore;
			mNewScore -= mNewScoreStep;
			if(mNewScore < 0) mNewScore = 0;
			char str[16] = "";
			sprintf(str, "%06ld", mScore);
			mScoreCounter->setCaption(String(str));

			sprintf(str, "%02d", mMushroomCount[mBaseColor]);
			mMushroomCounters[mBaseColor]->setCaption(String(str));
			mMushroomCounterFade[mBaseColor] = 1;

			// If we've unloaded the mushrooms, let's switch the level
			if(mMushroomCount[mBaseColor] == 0 && !mInSpecialLevel)
				mLevelChangeTimeout = 0.25f;

			SoundSystem::getSingleton().playSound("score");
		}
	}


	// Can we change the level?
	if(mLevelChangeTimeout > 0) {
		mLevelChangeTimeout -= delta;
		if(mLevelChangeTimeout <= 0)
			mChangeLevel = true;
	}
	if(mChangeLevel) {
		String level;
		if(!mGoingToSpecialLevel) level = mLastBase->getLevelFile();
		else level = "special_level.lua";
		if(!level.empty()) {
			mChangeLevel = false;
			mIsCounting = false;

			if(!mGoingToSpecialLevel) {
				mMinSpeed += 4.0f;
				mCurrentLevel++;

				char str[16] = "";
				sprintf(str, "%02d", mCurrentLevel);
				mLevelCounter[0]->setCaption(String(str));
				ScriptSystem::getSingleton().setScriptNumber("currentLevel", mCurrentLevel);
				SoundSystem::getSingleton().playSound("change_base");
			}
			else {
				mLevelCounter[0]->setCaption("??");
				mInSpecialLevel = true;
				mIsOnBase = true;
			}

			// Cancel the rotation
			Quaternion q = mCamera->getCamera()->getOrientation();
			Degree rot; Vector3 axis;
			q.ToAngleAxis(rot, axis);
			mNode->rotate(Vector3::UNIT_Z, Radian(rot));
            mCamera->resetRotation();

			gameApp->clearLevel();
			gameApp->loadLevel(level);
			mGoingToSpecialLevel = false;

			// Start playing the base sound
			mBaseSound = SoundSystem::getSingleton().playLoopedSound("base_hanging");
			if(mFlySound) mFlySound->stop();
			return;
		}
	}


	// Slowly accelerate
	if(!mIsOnBase && !mIsDead) {
		mSpeed += 1 * delta;

		// Trail waving
		mTrailWave += 35 * delta;
		mTrail->setInitialWidth(0, 3 + Math::Sin(mTrailWave)*0.75f);

		// The constant fly sound
		mConstantFlySoundCounter -= delta;
		if(mConstantFlySoundCounter < 0) {
			mConstantFlySoundCounter += 26.0f;
			mFlySound = SoundSystem::getSingleton().playSound("flight1");
		}


		// Do the fly random sounds
		mFlySoundCounter -= delta;
		if(mFlySoundCounter <= 0) {
			mFlySoundCounter = Math::RangeRandom(12,15);
			String sound = "flight" + StringConverter::toString(2 + rand()%11); 
			SoundSystem::getSingleton().playSound(sound);
		}
	}
	mFlareNode->roll(Radian(0.1f * delta));

	// Check for input
	if(!mIsDead && !mOnBase) {
		if(input->isKeyDown(OIS::KC_SPACE) || input->isKeyDown(OIS::KC_LCONTROL) || input->isKeyDown(OIS::KC_RCONTROL) || input->isKeyDown(OIS::KC_LSHIFT) || input->isKeyDown(OIS::KC_RSHIFT) || input->isKeyDown(OIS::KC_RETURN) || input->isMMouseDown()) {
			if(!mSpaceDown) {
				mSpaceDown = true;

				// Launch from the base
				if(mIsOnBase && !mIsCounting) {
					mIsOnBase = false;
					mSpeed = mMinSpeed;
					mFlareNode->setScale(1, 1, 1);
					SoundSystem::getSingleton().playSound("leave_base");
					if(mBaseSound) mBaseSound->stop();
					// Start the flying sound
					mFlySound = SoundSystem::getSingleton().playSound("flight1");
					mConstantFlySoundCounter = 26.0f;

					// Make the camera rotate if set on the script
					Real rot = ScriptSystem::getSingleton().getScriptNumber("levelRotation");
					mCamera->startRotation(rot);
				}
			}
		}
		else
			mSpaceDown = false;

		// Turn
		if(input->isKeyDown(OIS::KC_LEFT) || input->isLMouseDown())
			yaw(Radian(+5 * delta));
		else if(input->isKeyDown(OIS::KC_RIGHT) || input->isRMouseDown())
			yaw(Radian(-5 * delta));
	}

	// Keep moving
	if(!mIsOnBase) {
		// Move normally
		if(!mOnBase) {
			Vector3 dir = getDir();
			mCurrentSpeed = dir * mSpeed;
			mCurrentSpeed.z = 0;
		}
		else {
			// Move to the base
			Vector3 dir = mOnBase->getPosition() - getPosition();
			dir.z = 0;
			mCurrentSpeed = dir * 5.5f;
			setDir(dir.normalisedCopy());
		}

		// Check collisions
		if(!mIsDead && !mOnBase) {
			bool col = checkCollisions(getPosition(), mCurrentSpeed, delta);
			if(!col) {
				mSafePosition = getPosition();
				mNode->translate(mCurrentSpeed * delta);
			}
			else
				setPosition(mSafePosition);
		}
		else
			mNode->translate(mCurrentSpeed * delta);
		
		// Wrap around (non-visually)
		wrapAround();

		// Check if we're arrived onto the base
		if(mOnBase) {
			Vector3 dist = mOnBase->getPosition() - getPosition();
			dist.z = 0;
			if(dist.squaredLength() <= 0.005f) {
				mIsOnBase = true;
				Vector3 pos = mOnBase->getPosition(); pos.z = 0;
				setPosition(pos);
				mFlareNode->setPosition(pos);
				mFlareNode->setScale(0.75f, 0.75f, 0.75f);

			
				// Begin unloading the mushrooms
				mMushroomUnloadCounter = 0.33f;
				mIsCounting = true;
				mBaseColor = mOnBase->getColor();
				mOnBase = NULL;

				int f;
				for(f=0; f<NUM_MUSHROOMS; f++) {
					mMushroomCount[f] += mMushroomLevelCount[f];
					mMushroomLevelCount[f] = 0;
				}

				// Calculate the total score
				if(!mInSpecialLevel) {
					if(mMushroomCount[mBaseColor] > 0) {
						mNewScore = (long)(Math::Pow(1.4f, mMushroomCount[mBaseColor]) + 0.5f) * 15;
						mNewScoreStep = (long)((Real)mNewScore / (Real)mMushroomCount[mBaseColor] + 0.5f);
						mTotalMushrooms = mMushroomCount[mBaseColor];
					}
					else
						mChangeLevel = true;	// No mushrooms, just change the level
				}
				else {
					// Special level - unload all the mushrooms, starting from blue
					mBaseColor = BASE_BLUE;
					long totalScore = 0;
					mTotalMushrooms = 0;
					for(int i=0; i<NUM_MUSHROOMS; i++) {
						if(mMushroomCount[i] > 0)
							totalScore += (long)(Math::Pow(1.4f, mMushroomCount[i]) + 0.5f) * 15;
						mTotalMushrooms += mMushroomCount[i];
					}

					if(mTotalMushrooms > 0) {
						mNewScore = totalScore;
						mNewScoreStep = (long)((Real)mNewScore / (Real)mTotalMushrooms + 0.5f);
					}
					else {
						mChangeLevel = true;	// No mushrooms, just change the level
						mInSpecialLevel = false;
					}
				}

				// Make the scores visible
				mScoreFade = 1;
				ColourValue col = mScoreCounter->getColourTop();
				col.a = mScoreFade;
				mScoreCounter->setColourTop(col);
				col = mScoreCounter->getColourBottom();
				col.a = mScoreFade;
				mScoreCounter->setColourBottom(col);
				col = mPlaytimeCounter->getColourTop();
				col.a = mScoreFade;
				mPlaytimeCounter->setColourTop(col);

				// Make the level counter visible
				for(f=0; f<2; f++) {
					col = mLevelCounter[f]->getColourTop();
					col.a = mScoreFade;
					mLevelCounter[f]->setColourTop(col);
				}

				for(f=0; f<NUM_MUSHROOMS; f++) {
					char str[16] = "";
					sprintf(str, "%02d", mMushroomCount[f]);
					mMushroomCounters[f]->setCaption(String(str));
				}

				mOverlayTexFade[0]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);
				mOverlayTexFade[1]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);
			}
		}
	}

	// Wrap everything around as the player travels
	wrapEverything(this, mSceneMgr);
}


// Handle collisions
bool Player::preCollision(MovingObject *other) {
	if(other->getType() == OTYPE_MUSHROOM) {
		// Pick up the mushroom
		Mushroom *mush = static_cast<Mushroom*>(other);
		mush->pickUp();
		mush->setToBeDeleted();

		// Increase the counter
		int col = mush->getColor();
		mMushroomLevelCount[col]++;

		// Create a new mushroom
		if(!mInSpecialLevel) {
			Vector3 pos;
			ObjectSystem::getSingleton().findFreePosition(pos, getPosition(), other->getRadius(), 90);
			new Mushroom(mNextMushroomColor, "Mushroom" + StringConverter::toString(Mushroom::mushroomCounter), mSceneMgr, "Mushroom.mesh", pos);
			mNextMushroomColor++;
			if(mNextMushroomColor > NUM_MUSHROOMS-1)
				mNextMushroomColor = MUSHROOM_RED;
		}

		// Play a sound
		int s;
		if(Math::UnitRandom() < 0.5f) s = 1;
		else s = 2;
		SoundSystem::getSingleton().playSound("mushroom" + StringConverter::toString(s));
		return false;
	}
	else if(other->getType() == OTYPE_BASE) {
		if(mGoingToSpecialLevel) return false;

		// Base
		Base *base = static_cast<Base*>(other);

		// Start moving to the base if we're near enough
		Vector3 dist = base->getPosition() - getPosition(); dist.z = 0;
		Real d = dist.squaredLength();
		if(d <= 50 && base != mLastBase) {
			mOnBase = base;
			mOnBase->open();
			mLastBase = base;

			// Zoom in
			mCamera->setZoomIn(40);

			// Play the sound
			SoundSystem::getSingleton().playSound("base_enter");
		}
		else if(d > 50 && base == mLastBase)
			mLastBase = NULL;
		return false;
	}
	else if(other->getType() == OTYPE_ASTEROID) {
		if(mGoingToSpecialLevel) return false;

		// Collision with an asteroid -> player dies
		die();
	}

	return true;
}


void Player::afterCollision(MovingObject *other) {
	setDir(mCurrentSpeed.normalisedCopy());
	wrapAround();

	// Wrap the camera
	Vector3 pos = getPosition();
	Vector3 cPos = mCamera->getCamera()->getPosition();
	Real dx = pos.x - cPos.x;
	Real dy = pos.y - cPos.y;

	if(dx > playfieldWidth)
		cPos.x += playfieldWidth*2;
	else if(dx < -playfieldWidth)
		cPos.x -= playfieldWidth*2;
	if(dy > playfieldHeight)
		cPos.y += playfieldHeight*2;
	else if(dy < -playfieldHeight)
		cPos.y -= playfieldHeight*2;

	mCamera->getCamera()->setPosition(cPos);
}


// Player dies
void Player::die() {
	mNode->setVisible(false);
	mFlareNode->setVisible(false);
	mFlareNode->setScale(0.75f, 0.75f, 0.75f);
	mIsDead = true;
	mSpeed = 0.0f;
	mCurrentSpeed *= -0.1f;
	//mCurrentSpeed.z = 0;
	mDeathCount = 3.5f;
	//mTrail->setVisible(false);

	try {
		// Create the effects
		Effect *effect = new Effect(EFFECT_PLAYER_DEATH, "PlayerDeathEffect", mSceneMgr, "", getPosition());
		effect->createParticles("PlayerDeath");
		effect->setCurrentSpeed(mCurrentSpeed);
		effect->setDuration(4.0f);

		// Create the sparks
		effect = new Effect(EFFECT_PLAYER_DEATH, "PlayerDeathEffectSparks", mSceneMgr, "", getPosition());
		effect->createParticles("PlayerDeathSparks");
		effect->setDuration(2.0f);
		effect->setCurrentSpeed(mCurrentSpeed);
		effect->createLight(ColourValue(0.4f, 0.75f, 1));
	}
	catch(...) {
		// Ignore..
	}

	// Clear the mushrooms
	for(int f=0; f<NUM_MUSHROOMS; f++) {
		mMushroomLevelCount[f] = 0;
		char str[16] = "";
		sprintf(str, "%02d", mMushroomCount[f]);
		mMushroomCounters[f]->setCaption(String(str));
	}

	// Play the death sound
	SoundSystem::getSingleton().playSound("death");
	if(mFlySound) mFlySound->stop();

	mOverlayTexFade[0]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 1);
	mOverlayTexFade[1]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, 1);
	OverlayManager::getSingleton().getByName("DisplayOverlay")->hide();
}


// Respawn the player
void Player::respawn() {
	mIsDead = false;
	mNode->setVisible(true);
	mFlareNode->setVisible(true);
	mTrail->setVisible(true);

	Vector3 pos;
	Base *base = static_cast<Base*>(ObjectSystem::getSingleton().findObject(ScriptSystem::getSingleton().getScriptString("playerHomeBase")));
	pos = base->getPosition();
	pos.z = 0;

	mIsOnBase = true;
	mLastBase = base;
	moveTo(pos);

	// Make the scores visible
	mScoreFade = 1;
	ColourValue col = mScoreCounter->getColourTop();
	col.a = mScoreFade;
	mScoreCounter->setColourTop(col);
	col = mScoreCounter->getColourBottom();
	col.a = mScoreFade;
	mScoreCounter->setColourBottom(col);
	col = mPlaytimeCounter->getColourTop();
	col.a = mScoreFade;
	mPlaytimeCounter->setColourTop(col);

	// Make the level counter visible
	int f;
	for(f=0; f<2; f++) {
		col = mLevelCounter[f]->getColourTop();
		col.a = mScoreFade;
		mLevelCounter[f]->setColourTop(col);
	}

	mOverlayTexFade[0]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);
	mOverlayTexFade[1]->setAlphaOperation(LBX_MODULATE, LBS_TEXTURE, LBS_MANUAL, 1.0f, mScoreFade);

	for(f=0; f<NUM_MUSHROOMS; f++)
		mMushroomCounterFade[f] = 1;

	// Start the base sound
	mBaseSound = SoundSystem::getSingleton().playLoopedSound("base_hanging");
}


// Move the player (and camera, etc.) to a certain position
void Player::moveTo(const Vector3 &pos) {
	setPosition(pos);
	mFlareNode->setPosition(pos);
	mCamera->getCamera()->setPosition(pos);
	mCamera->setZoomOut(50);
	mChangeLevel = false;

	// Fix the trail
	for(unsigned int f=0; f<mTrail->getMaxChainElements(); f++) {
		BillboardChain::Element bcElem = mTrail->getChainElement(0, f);
		bcElem.position = pos;
		mTrail->updateChainElement(0, f, bcElem);
	}
}


// Wrap around
void Player::wrapAround() {
	// Wrap around (non-visually)
	Vector3 pos = getPosition();
	bool wrapped = false;
	if(pos.x < -playfieldWidth) {
		pos.x += playfieldWidth*2;
		Vector3 camPos = mCamera->getCamera()->getPosition();
		camPos.x += playfieldWidth*2;
		mCamera->getCamera()->setPosition(camPos);
		wrapped = true;
	}
	else if(pos.x > playfieldWidth) {
		pos.x -= playfieldWidth*2;
		Vector3 camPos = mCamera->getCamera()->getPosition();
		camPos.x -= playfieldWidth*2;
		mCamera->getCamera()->setPosition(camPos);
		wrapped = true;
	}
	if(pos.y < -playfieldHeight) {
		pos.y += playfieldHeight*2;
		Vector3 camPos = mCamera->getCamera()->getPosition();
		camPos.y += playfieldHeight*2;
		mCamera->getCamera()->setPosition(camPos);
		wrapped = true;
	}
	else if(pos.y > playfieldHeight) {
		pos.y -= playfieldHeight*2;
		Vector3 camPos = mCamera->getCamera()->getPosition();
		camPos.y -= playfieldHeight*2;
		mCamera->getCamera()->setPosition(camPos);
		wrapped = true;
	}
	setPosition(pos);
	mFlareNode->setPosition(pos);

	// Fix the trail
	if(wrapped) {
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
}




// Wrap all the scenenodes around which are too far from the player
void wrapEverything(Player *player, SceneManager *sceneMgr) {
	Vector3 pPos = player->getPosition();

	Real w = playfieldWidth;
	Real h = playfieldHeight;

	// Wrap all the moving objects (enemies, ...)
	ObjectMapType::const_iterator i;
	ObjectSystem *ob = ObjectSystem::getSingletonPtr();
	for(i=ob->getFirst(); i != ob->getLast(); ++i) {
		Vector3 oPos = (*i).second->getPosition();

		Real dx = pPos.x - oPos.x;
		Real dy = pPos.y - oPos.y;

		if(dx > w)
			oPos.x += w*2;
		else if(dx < -w)
			oPos.x -= w*2;
		if(dy > h)
			oPos.y += h*2;
		else if(dy < -h)
			oPos.y -= h*2;

		(*i).second->setPosition(oPos);
		(*i).second->updateAfterWrap();
	}

	try {
		// Wrap the background stars
		unsigned int f;
		for(f=0; f<4; f++) {
			SceneNode *stars = sceneMgr->getSceneNode("BackgroundStars" + StringConverter::toString(f+1));
			Vector3 sPos = stars->getPosition();

			Real dx = pPos.x - sPos.x;
			Real dy = pPos.y - sPos.y;

			if(dx > w)
				sPos.x += w*2;
			else if(dx < -w)
				sPos.x -= w*2;
			if(dy > h)
				sPos.y += h*2;
			else if(dy < -h)
				sPos.y -= h*2;

			stars->setPosition(sPos);
		}


		// Wrap the background nebulae
		for(f=0; f<(unsigned int)nebulaCount; f++) {
			SceneNode *neb = sceneMgr->getSceneNode("NebulaNode" + StringConverter::toString(f));
			Vector3 nPos = neb->getPosition();

			Real dx = pPos.x - nPos.x;
			Real dy = pPos.y - nPos.y;

			if(dx > w)
				nPos.x += w*2;
			else if(dx < -w)
				nPos.x -= w*2;
			if(dy > h)
				nPos.y += h*2;
			else if(dy < -h)
				nPos.y -= h*2;

			neb->setPosition(nPos);
		}
	}
	catch(...) {
		// Ignore..
	}
}


