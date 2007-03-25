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

#include "bground.h"
#include "game.h"
#include "scriptsystem.h"



// Total amount of the nebulae
int nebulaCount = 10;
static Rectangle2D *bgRect;

// Create the background nebulae
void createBackground(SceneManager *sceneMgr) {
	
	// Get the background image
	String bgImage = ScriptSystem::getSingleton().getScriptString("backgroundImage");
	MaterialPtr mat = MaterialManager::getSingleton().getByName("Background");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName(bgImage);

	// Get the nebula textures, and set their colors
	ColourValue col;
	col.r = ScriptSystem::getSingleton().getScriptNumber("nebula1_R");
	col.g = ScriptSystem::getSingleton().getScriptNumber("nebula1_G");
	col.b = ScriptSystem::getSingleton().getScriptNumber("nebula1_B");
	mat = MaterialManager::getSingleton().getByName("BackgroundNebula1");
	TextureUnitState *tu = mat->getTechnique(0)->getPass(0)->getTextureUnitState(1);
	tu->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, col, col);

	col.r = ScriptSystem::getSingleton().getScriptNumber("nebula2_R");
	col.g = ScriptSystem::getSingleton().getScriptNumber("nebula2_G");
	col.b = ScriptSystem::getSingleton().getScriptNumber("nebula2_B");
	mat = MaterialManager::getSingleton().getByName("BackgroundNebula2");
	tu = mat->getTechnique(0)->getPass(0)->getTextureUnitState(1);
	tu->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, col, col);

	col.r = ScriptSystem::getSingleton().getScriptNumber("nebula3_R");
	col.g = ScriptSystem::getSingleton().getScriptNumber("nebula3_G");
	col.b = ScriptSystem::getSingleton().getScriptNumber("nebula3_B");
	mat = MaterialManager::getSingleton().getByName("BackgroundNebula3");
	tu = mat->getTechnique(0)->getPass(0)->getTextureUnitState(1);
	tu->setColourOperationEx(LBX_MODULATE, LBS_CURRENT, LBS_MANUAL, col, col);


	// Create background rectangle covering the whole screen
	bgRect = new Rectangle2D(true);
	// Flipping and mirroring
	Real bgx = rand()%100 < 50 ? -1.0f : 1.0f;
	Real bgy = rand()%100 < 50 ? -1.0f : 1.0f;
	bgRect->setCorners(-bgx, bgy, bgx, -bgy);
	bgRect->setMaterial("Background");

	// Render the background before everything else
	bgRect->setRenderQueueGroup(RENDER_QUEUE_SKIES_EARLY);
	AxisAlignedBox box; box.setInfinite();
	bgRect->setBoundingBox(box);

	// Attach background to the scene
	SceneNode* node = sceneMgr->getRootSceneNode()->createChildSceneNode("Background");
	node->attachObject(bgRect);


	// Background stars
	int starsCount = (int)ScriptSystem::getSingleton().getScriptNumber("starsCount");
	ManualObject *stars[4];
	for(int ii=0; ii<4; ii++) {
		stars[ii] = sceneMgr->createManualObject("BackgroundStars" + StringConverter::toString(ii+1));
		stars[ii]->begin("BackgroundStars", RenderOperation::OT_POINT_LIST);
		for(int j=0; j<starsCount; j++) {
			Vector3 pos;
			pos.x = Math::RangeRandom(-playfieldWidth, playfieldWidth);
			pos.y = Math::RangeRandom(-playfieldHeight, playfieldHeight);
			pos.z = Math::RangeRandom(-140, -5);
			stars[ii]->position(pos);

			Real c = Math::RangeRandom(0.1f, 1.0f);
			stars[ii]->colour(c, c, c);
			stars[ii]->index(j);
		}
		stars[ii]->end();
		stars[ii]->setRenderQueueGroup(RENDER_QUEUE_1);
		stars[ii]->setCastShadows(false);
	}

	// Create four groups of stars
	SceneNode *starsNode = sceneMgr->getRootSceneNode()->createChildSceneNode("BackgroundStars1");
	starsNode->attachObject(stars[0]);
	starsNode->translate(-playfieldWidth/2, -playfieldHeight/2, 0);

	starsNode = sceneMgr->getRootSceneNode()->createChildSceneNode("BackgroundStars2");
	starsNode->attachObject(stars[1]);
	starsNode->translate(+playfieldWidth/2, -playfieldHeight/2, 0);

	starsNode = sceneMgr->getRootSceneNode()->createChildSceneNode("BackgroundStars3");
	starsNode->attachObject(stars[2]);
	starsNode->translate(+playfieldWidth/2, +playfieldHeight/2, 0);

	starsNode = sceneMgr->getRootSceneNode()->createChildSceneNode("BackgroundStars4");
	starsNode->attachObject(stars[3]);
	starsNode->translate(-playfieldWidth/2, +playfieldHeight/2, 0);


	// Create the nebulae
	nebulaCount = (int)ScriptSystem::getSingleton().getScriptNumber("nebulaCount");
	for(int f=0; f<nebulaCount; f++) {
		Entity *ent = sceneMgr->createEntity("BackgroundNebula" + StringConverter::toString(f), "Plane.mesh");
		ent->setMaterialName("BackgroundNebula" + StringConverter::toString(f%3 + 1));
		ent->setCastShadows(false);
		ent->setRenderQueueGroup(RENDER_QUEUE_SKIES_EARLY);

		Vector3 pos;
		pos.x = Math::RangeRandom(-playfieldWidth, playfieldWidth);
		pos.y = Math::RangeRandom(-playfieldHeight, playfieldHeight);
		pos.z = Math::RangeRandom(-30, -5);

		SceneNode *node = sceneMgr->getRootSceneNode()->createChildSceneNode("NebulaNode" + StringConverter::toString(f), pos);
		node->attachObject(ent);

		Real s = Math::RangeRandom(25, 55);
		node->scale(s,s,s);
		node->roll(Degree(Math::RangeRandom(0,360)));
		node->yaw(Degree(Math::RangeRandom(-15,15)));
	}
}


// Destroy the background nebulae
void destroyBackground(SceneManager *sceneMgr) {
	if(bgRect)
		delete bgRect;

	MaterialPtr mat = MaterialManager::getSingleton().getByName("Background");
	mat->unload();

	sceneMgr->destroySceneNode("Background");
	int f;
	for(f=0; f<4; f++) {
		SceneNode *node = sceneMgr->getSceneNode("BackgroundStars" + StringConverter::toString(f+1));
		node->removeAndDestroyAllChildren();
		node->detachAllObjects();
		//sceneMgr->destroyEntity("BackgroundStarsEntity" + StringConverter::toString(f+1));
		sceneMgr->destroyManualObject("BackgroundStars" + StringConverter::toString(f+1));
		sceneMgr->destroySceneNode(node->getName());
	}

	for(f=0; f<nebulaCount; f++) {
		SceneNode *node = sceneMgr->getSceneNode("NebulaNode" + StringConverter::toString(f));
		node->removeAndDestroyAllChildren();
		node->detachAllObjects();

		sceneMgr->destroyEntity("BackgroundNebula" + StringConverter::toString(f));
		sceneMgr->destroySceneNode(node->getName());
	}
}
