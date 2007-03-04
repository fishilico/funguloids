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

#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#include "lua.hpp"

#include <Ogre.h>
#include <OgreSingleton.h>
#include <vector>

using namespace Ogre;
using namespace std;


class Player;

// ScriptSystem
class ScriptSystem : public Singleton<ScriptSystem> {
private:
	lua_State *mLuaVM;					// Lua VM
	Player *mPlayer;					// Player object
	SceneManager *mSceneMgr;			// Ogre scene manager

public:
	ScriptSystem(SceneManager *mgr, const char *startup_file = 0);
	~ScriptSystem();

	static ScriptSystem &getSingleton();
	static ScriptSystem *getSingletonPtr();

	bool executeScript(const char *file);
	bool executeString(const char *str);

	void setPlayer(Player *player) { mPlayer = player; }
	Player *getPlayer() const { return mPlayer; }

	Real getScriptNumber(const char *value);
	String getScriptString(const char *value);
	void setScriptNumber(const char *value, Real number) {
		lua_pushnumber(mLuaVM, number);
		lua_setglobal(mLuaVM, value);
	}

	void destroyScriptLights();
	SceneManager *getSceneMgr() const { return mSceneMgr; }
};


#endif
