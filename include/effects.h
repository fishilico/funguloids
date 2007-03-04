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

#ifndef EFFECTS_H
#define EFFECTS_H

#include <Ogre.h>
#include "movingobject.h"
using namespace Ogre;

// Object type
const int OTYPE_EFFECT = OTYPE_MOVING_BASIC + 6;

// Effect types
enum {
	EFFECT_MUSHROOM_CATCH = 0,
	EFFECT_PLAYER_DEATH,
	EFFECT_WHIRLER_CATCH
};


class Effect : public MovingObject {
private:
	Real mEffectDuration;					// Duration of the effect
	Real mEffectTime;						// How long has it been active?
	int mEffectType;						// Effect type
	ParticleSystem *mParticleSystem;		// Particle system for the effect
	Light *mLight;							// Light
	ColourValue mLightColor;				// Light color

public:
	Effect(int effectType, const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed=true);
	~Effect();

	void move(Real delta);
	void createParticles(const String &script);
	void createLight(const ColourValue &col);
	void setDuration(Real d) { mEffectDuration = d; }
};


void createEffectLights(SceneManager *sceneMgr);
void destroyEffectLights(SceneManager *sceneMgr);

#endif
