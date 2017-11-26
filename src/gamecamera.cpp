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

#include <Ogre.h>
#include "game.h"
#include "gamecamera.h"

const Real cameraAreaWidth = playfieldWidth - 63.5f;
const Real cameraAreaHeight = playfieldHeight - 48.0f;


void GameCamera::followTarget(Real delta, Real speed) {
	Vector3 targetPos = mTarget->getPosition();
	Vector3 camPos = mCamNode->getPosition();
	camPos.z = targetPos.z;
	Vector3 dir = targetPos - camPos;


	// Move towards the target
	Vector3 mov = dir * delta * speed;
	mov.z = 0;
	Vector3 pos = mCamNode->getPosition() + mov;

	// Rotation
	if(mRotating > 0) {
		mCamera->rotate(Vector3::UNIT_Z, Radian(Degree(delta * -mRotating)));
	}

	// Zoom
	if(mCamZoom > 0) {
		Real d = pos.z - mCamZoom;
		pos.z -= d * delta * (mZoomIn ? 2.5f : 1.0f);
		if(mZoomIn && pos.z < mCamZoom) {
			pos.z = mCamZoom;
			mCamZoom = 0;
		}
		else if(!mZoomIn && pos.z > mCamZoom) {
			pos.z = mCamZoom;
			mCamZoom = 0;
		}
	}

	mCamNode->setPosition(pos);
}
