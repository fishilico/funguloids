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

#include "effects.h"
#include "game.h"


// Effect durations
static const Real effectDurations[] = { 3.0f, 5.0f, 3.0f };


// Effect lights
#define MAX_EFFECT_LIGHTS		8
static Light *effectLights[MAX_EFFECT_LIGHTS];
static int currentEffectLight = 0;


// Effect constructor
Effect::Effect(int effectType, const String &name, SceneManager *sceneMgr, const String &mesh, const Vector3 &pos, bool managed) : MovingObject(name, sceneMgr, mesh, pos, managed) {
	mType = OTYPE_EFFECT;
	mEffectType = effectType;
	mEffectDuration = effectDurations[effectType];
	mEffectTime = 0;
	
	mLight = NULL;
	mParticleSystem = NULL;
}


// Effect destructor
Effect::~Effect() {
	if(mParticleSystem) {
		mNode->detachObject(mParticleSystem);
		mSceneMgr->destroyParticleSystem(mParticleSystem);
	}

	if(mLight) {
		mNode->detachObject(mLight);
		mLight->setVisible(false);
	}
}


// Move (update) the effect
void Effect::move(Real delta) {
	// Update the time
	mEffectTime += delta;
	if(mEffectTime >= mEffectDuration) setToBeDeleted();

	if(mLight) {
		Real l = 1.0f - mEffectTime / mEffectDuration;
		mLight->setDiffuseColour(l*mLightColor.r, l*mLightColor.g, l*mLightColor.b);
		mLight->setSpecularColour(l, l, l);
	}
	

	// Move
	mNode->translate(mCurrentSpeed * delta);
}


// Create the particle system from a script
void Effect::createParticles(const String &script) {
	mParticleSystem = mSceneMgr->createParticleSystem(mName, script);
	mNode->attachObject(mParticleSystem);
}


// Create a light
void Effect::createLight(const ColourValue &col) {
	String dynlight = GameApplication::mGameConfig->GetValue("graphics", "dynamic_lights", "on");
	if(dynlight.compare("off") == 0) return;

	String shadows = GameApplication::mGameConfig->GetValue("graphics", "dynamic_light_shadows", "on");
	bool on = shadows.compare("off") != 0;

	// Retrieve the effect light pointer
	currentEffectLight++;
	if(currentEffectLight > MAX_EFFECT_LIGHTS-1) currentEffectLight = 0;
	mLight = effectLights[currentEffectLight];

	// Set it up
	mLightColor = col;
	mLight->setDiffuseColour(col);
	mLight->setSpecularColour(ColourValue::White);
	mLight->setVisible(true);
	mLight->setCastShadows(on);
	mNode->attachObject(mLight);
}


// Create the effect light pool
void createEffectLights(SceneManager *sceneMgr) {
	for(int f=0; f<MAX_EFFECT_LIGHTS; f++) {
		effectLights[f] = sceneMgr->createLight("EffectLight" + StringConverter::toString(f+1));
		effectLights[f]->setType(Light::LT_POINT);
		effectLights[f]->setVisible(false);
	}
}


// Destroy the effect lights
void destroyEffectLights(SceneManager *sceneMgr) {
	for(int f=0; f<MAX_EFFECT_LIGHTS; f++) {
		sceneMgr->destroyLight(effectLights[f]);
	}
}
