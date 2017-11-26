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

#ifndef CAMERA_H
#define CAMERA_H

class GameCamera {
private:
	Camera *mCamera;			// Pointer to the Ogre camera
	SceneNode *mCamNode;			// Scene node for the camera
	SceneNode *mTarget;			// Target node
	Real mCamZ;					// Camera Z coordinate (doesn't change)
	Real mCamZoom;				// Zoom target
	bool mZoomIn;				// Zoom in or out?
	Real mRotating;				// Is the camera rotating?
	Quaternion mOriginalOr;		// Original orientation

public:
	GameCamera(Camera *cam, SceneNode* camNode) {
		mCamera = cam;
		mCamNode = camNode;
		mTarget = NULL;
		mCamZ = camNode->getPosition().z;
		mZoomIn = false;
		mRotating = 0;
		mCamZoom = 0;
		mOriginalOr = camNode->getOrientation();
	}

	void followTarget(Real delta, Real speed);
	void setTarget(SceneNode *node) {
		mTarget = node;
		Vector3 pos = mTarget->getPosition();
		pos.z = mCamZ;
		mCamNode->setPosition(pos);
	}

	Real getRotation() const { return mRotating; }

	void startRotation(Real start) {
		mRotating = start;
	}

	void addRotation(Real add) {
		mRotating += add;
	}

	void resetRotation() {
		mRotating = 0;
		mCamNode->setOrientation(mOriginalOr);
	}

	void setZoomIn(Real zpos) {
		mCamZoom = zpos;
		//Vector3 pos = mCamera->getPosition();
		//pos.z = mCamZ;
		//mCamera->setPosition(pos);
		mZoomIn = true;
	}

	void setZoomOut(Real zpos) {
		mCamZoom = mCamZ;
		Vector3 pos = mCamNode->getPosition();
		pos.z = zpos;
		mCamNode->setPosition(pos);
		mZoomIn = false;
	}

	Camera *getCamera() const { return mCamera; }
	SceneNode *getCamNode() const { return mCamNode; }
};

#endif
