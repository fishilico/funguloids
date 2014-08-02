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

#include "scriptsystem.h"
#include "game.h"
#include "asteroid.h"
#include "objectsystem.h"
#include "mushroom.h"
#include "base.h"
#include "soundsystem.h"
#include "whirler.h"
#include "ballworm.h"
#include "blackhole.h"
#include "tentacles.h"
#include "playlist.h"
#include "mpakogre.h"
#include <vector>

template<> ScriptSystem* Singleton<ScriptSystem>::msSingleton = 0;


// Log error messages
void luaError(const String &str) {
	LogManager::getSingleton().logMessage("ScriptSystem error: " + str);
}


// This keeps track of the lights created in script
static std::vector<Light*> scriptLights;


// File locator for script files
// (based on wiki example at http://www.ogre3d.org/wiki/index.php/File_SoundManager.cpp)
class ScriptLocator : public ResourceGroupManager {
public:
	ScriptLocator() {}
	~ScriptLocator() {}

	Archive *findScript(String &filename) {
		ResourceGroup *grp = getResourceGroup("General");
		if(!grp)
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate a resource group called 'General'", "ResourceGroupManager::openResource");
		
		OGRE_LOCK_MUTEX(grp->OGRE_AUTO_MUTEX_NAME); // lock group mutex
		ResourceLocationIndex::iterator rit = grp->resourceIndexCaseSensitive.find(filename);
		if(rit != grp->resourceIndexCaseSensitive.end()) {
			// Found in the index
			Archive *fileArchive = rit->second;
			filename = fileArchive->getName() + "/" + filename;
			return fileArchive;
		}

		return NULL;
	}
};



////////////////////////////////////////////
// Glue code between C++ and LUA
////////////////////////////////////////////

// Allows us to log messages from the scripts
static int LogMessage(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 1) {
		luaError(String("LogMessage():\nThis function expects a string as an argument!"));
		return 0;
	}

	// Get the string
	const char *str = lua_tostring(L, 1);
	LogManager::getSingleton().logMessage("ScriptSystem: " + String(str));
	return 0;
}


// Run a script
static int RunScript(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 1) {
		luaError(String("RunScript():\nThis function expects a file name as an argument!"));
		return 0;
	}

	// Get the string
	const char *str = lua_tostring(L, 1);
	ScriptSystem::getSingleton().executeScript(str);
	return 0;
}


// Create an asteroid
static int CreateAsteroid(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 5) {
		luaError(String("CreateAsteroid():\nThis function expects several arguments!: (Name, Mesh filename, X position, Y position, Scale [, Material name])"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	const char *mesh = lua_tostring(L, 2);
	Real xpos = lua_tonumber(L, 3);
	Real ypos = lua_tonumber(L, 4);
	Real scale = lua_tonumber(L, 5);
	const char *mat = "AsteroidMat";
	if(num == 6) mat = lua_tostring(L, 6);

	// Create
	Vector3 pos(xpos, ypos, 0);
	Asteroid *roid = new Asteroid(String(name), ScriptSystem::getSingleton().getSceneMgr(), String(mesh), String(mat), pos);
	roid->setScale(scale, scale, scale);
	roid->setMass(scale * 10.0f);
	//roid->getEntity()->setNormaliseNormals(true);
	return 0;
}


// Create a mushroom
static int CreateMushroom(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 4) {
		luaError(String("CreateMushroom():\nThis function expects several arguments!: (Name, Color[0-3], X position, Y position)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	int color = (int)lua_tonumber(L, 2);
	Real xpos = lua_tonumber(L, 3);
	Real ypos = lua_tonumber(L, 4);

	// Create
	Vector3 pos(xpos, ypos, 0);
	new Mushroom(color, String(name), ScriptSystem::getSingleton().getSceneMgr(), "Mushroom.mesh", pos);
	return 0;
}


// Create a whirler
static int CreateWhirler(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 3) {
		luaError(String("CreateWhirler():\nThis function expects several arguments!: (Name, X position, Y position)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	Real xpos = lua_tonumber(L, 2);
	Real ypos = lua_tonumber(L, 3);

	// Create
	Vector3 pos(xpos, ypos, 0);
	new Whirler(String(name), ScriptSystem::getSingleton().getPlayer(), ScriptSystem::getSingleton().getSceneMgr(), "Whirler.mesh", pos);
	return 0;
}


// Create a ball worm
static int CreateBallWorm(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 6) {
		luaError(String("CreateBallWorm():\nThis function expects several arguments!: (Name, X position, Y position, Orbit radius, Rotation speed/direction, Ball angle step)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	Real xpos = lua_tonumber(L, 2);
	Real ypos = lua_tonumber(L, 3);
	Real orad = lua_tonumber(L, 4);
	Real rotSpeed = lua_tonumber(L, 5);
	Real angleStep = lua_tonumber(L, 6);

	// Create
	Vector3 pos(xpos, ypos, 0);
	new BallWorm(orad, rotSpeed, angleStep, ScriptSystem::getSingleton().getPlayer(), String(name), ScriptSystem::getSingleton().getSceneMgr(), pos);
	return 0;
}


// Create a base
static int CreateBase(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 4) {
		luaError(String("CreateBase():\nThis function expects several arguments!: (Name, Color[0-3], X position, Y position [, Level filename])"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	int color = (int)lua_tonumber(L, 2);
	Real xpos = lua_tonumber(L, 3);
	Real ypos = lua_tonumber(L, 4);
	const char *level;
	if(num == 5) level = lua_tostring(L, 5);
	else level = "";

	// Create
	Vector3 pos(xpos, ypos, -4.0f);
	new Base(color, String(name), String(level), ScriptSystem::getSingleton().getSceneMgr(), "Base.mesh", pos);
	return 0;
}


// Create a black hole
static int CreateBlackHole(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 5) {
		luaError(String("CreateBlackHole():\nThis function expects several arguments!: (Name, X position, Y position, Effective Distance, Effect multiplier)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	Real xpos = lua_tonumber(L, 2);
	Real ypos = lua_tonumber(L, 3);
	Real dist = lua_tonumber(L, 4);
	Real effect = lua_tonumber(L, 5);

	// Create
	Vector3 pos(xpos, ypos, 0);
	new BlackHole(dist, effect, String(name), ScriptSystem::getSingleton().getPlayer(), ScriptSystem::getSingleton().getSceneMgr(), "Plane.mesh", pos);
	return 0;
}


// Create tentacles
static int CreateTentacles(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 3) {
		luaError(String("CreateTentacles():\nThis function expects several arguments!: (Name, X position, Y position)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	Real xpos = lua_tonumber(L, 2);
	Real ypos = lua_tonumber(L, 3);

	// Create
	Vector3 pos(xpos, ypos, -12);
	new Tentacles(String(name), ScriptSystem::getSingleton().getSceneMgr(), "Tentacle.mesh", pos);
	return 0;
}


// Create a light
static int CreateLight(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 7) {
		luaError(String("CreateLight():\nThis function expects several arguments!: (Name, Direction X, Dir Y, Dir Z, Color R, Col G, Col B)"));
		return 0;
	}

	// Get the params
	const char *name = lua_tostring(L, 1);
	Vector3 dir;
	dir.x = lua_tonumber(L, 2);
	dir.y = lua_tonumber(L, 3);
	dir.z = lua_tonumber(L, 4);
	dir.normalise();

	ColourValue col;
	col.r = lua_tonumber(L, 5);
	col.g = lua_tonumber(L, 6);
	col.b = lua_tonumber(L, 7);

	// Create
	Light *light = ScriptSystem::getSingleton().getSceneMgr()->createLight(String(name));
	light->setType(Light::LT_DIRECTIONAL);
	light->setCastShadows(false);
	light->setDiffuseColour(col);
	light->setSpecularColour(ColourValue::White);
	light->setDirection(dir);
	scriptLights.push_back(light);

	return 0;
}


// Set the ambient light
static int SetAmbientLight(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 3) {
		luaError(String("SetAmbientLight():\nThis function expects several arguments!: (Color R, Col G, Col B)"));
		return 0;
	}

	// Get the params
	ColourValue col;
	col.r = lua_tonumber(L, 1);
	col.g = lua_tonumber(L, 2);
	col.b = lua_tonumber(L, 3);

	// Set
	ScriptSystem::getSingleton().getSceneMgr()->setAmbientLight(col);
	return 0;
}


// Get a free (random) position, away from the specified position
static int GetFreePosition(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 3) {
		luaError(String("GetFreePosition():\nThis function expects several arguments!: (Player X, Player Y, Radius [, Min distance])"));
		return 0;
	}

	// Get the params
	Real px = lua_tonumber(L, 1);
	Real py = lua_tonumber(L, 2);
	Real rad = lua_tonumber(L, 3);
	Vector3 pos;
	if(num == 3)
		ObjectSystem::getSingleton().findFreePosition(pos, Vector3(px, py, 0), rad);
	else {
		Real dist = lua_tonumber(L, 4);
		ObjectSystem::getSingleton().findFreePosition(pos, Vector3(px, py, 0), rad, dist);
	}

	// Return the position
	lua_pushnumber(L, pos.x);
	lua_pushnumber(L, pos.y);
	return 2;
}


// Play a music file
static int PlayMusic(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 1) {
		luaError(String("PlayMusic():\nThis function expects a file name as an argument!"));
		return 0;
	}

	// Get the string
	const char *str = lua_tostring(L, 1);
	SoundSystem::getSingleton().playMusic(str);
	return 0;
}


// Add files to the playlist
static int AddToPlaylist(lua_State *L) {
	// Number of arguments
	int num = lua_gettop(L);
	if(num < 1) {
		luaError(String("AddToPlaylist():\nThis function expects file name(s) as an argument! Wildcards supported."));
		return 0;
	}

	// Get the string
	const char *str = lua_tostring(L, 1);
	addToPlaylist(String(str));
	return 0;
}


// Shuffle the playlist
static int ShufflePlaylist(lua_State *L) {
	shufflePlaylist();
	return 0;
}


// Get the next song from the playlist
static int GetNextSong(lua_State *L) {
	// Return the next song
	String song = getNextSong();
	lua_pushstring(L, song.c_str());
	return 1;
}


// Get the music directory
// $HOME/music/ on Linux, the ./music/ on Windows
static int GetMusicDir(lua_State *L) {
	// Return the music directory
	String dir;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32	// For Windows
	dir = "music/";
#else							// For Linux/Unix/etc.
	extern String getFunguloidsDir();
	dir = getFunguloidsDir() + "music/";
#endif

	lua_pushstring(L, dir.c_str());
	return 1;	
}

////////////////////////////////////////////



// ScriptSystem constructor
ScriptSystem::ScriptSystem(SceneManager *mgr, const char *startup_file) {
	assert(mgr);
	mSceneMgr = mgr;
	scriptLights.clear();

	LogManager::getSingleton().logMessage("Using " + String(LUA_VERSION));
	// Create the script engine
	mLuaVM = luaL_newstate();
	if(!mLuaVM)
		LogManager::getSingleton().logMessage(LML_CRITICAL, "Unable to create the script engine!");

	// Initialise Lua
	luaopen_base(mLuaVM);
	luaopen_string(mLuaVM);
	luaopen_math(mLuaVM);


	// Register our functions
	lua_register(mLuaVM, "LogMessage", LogMessage);
	lua_register(mLuaVM, "RunScript", RunScript);
	lua_register(mLuaVM, "CreateAsteroid", CreateAsteroid);
	lua_register(mLuaVM, "CreateMushroom", CreateMushroom);
	lua_register(mLuaVM, "CreateWhirler", CreateWhirler);
	lua_register(mLuaVM, "CreateBallWorm", CreateBallWorm);
	lua_register(mLuaVM, "CreateBase", CreateBase);
	lua_register(mLuaVM, "CreateBlackHole", CreateBlackHole);
	lua_register(mLuaVM, "CreateTentacles", CreateTentacles);
	lua_register(mLuaVM, "CreateLight", CreateLight);
	lua_register(mLuaVM, "SetAmbientLight", SetAmbientLight);
	lua_register(mLuaVM, "GetFreePosition", GetFreePosition);
	lua_register(mLuaVM, "PlayMusic", PlayMusic);
	lua_register(mLuaVM, "AddToPlaylist", AddToPlaylist);
	lua_register(mLuaVM, "ShufflePlaylist", ShufflePlaylist);
	lua_register(mLuaVM, "GetNextSong", GetNextSong);
	lua_register(mLuaVM, "GetMusicDir", GetMusicDir);
	LogManager::getSingleton().logMessage("ScriptSystem created.");

	// Execute the start up script, if specified
	if(startup_file) {
		executeScript(startup_file);
	}
}


// ScriptSystem destructor
ScriptSystem::~ScriptSystem() {
	if(mLuaVM)
		lua_close(mLuaVM);
	LogManager::getSingleton().logMessage("ScriptSystem destroyed.");
}


// Get a global number from the script
Real ScriptSystem::getScriptNumber(const char *value) {
	lua_getglobal(mLuaVM, value);
	lua_Number r = lua_tonumber(mLuaVM, -1);
	lua_pop(mLuaVM, 1);

	return (Real)r;
}


// Get a global string from the script
String ScriptSystem::getScriptString(const char *value) {
	lua_getglobal(mLuaVM, value);
	String r(lua_tostring(mLuaVM, -1));
	lua_pop(mLuaVM, 1);

	return r;
}


// Destroy the script created lights
void ScriptSystem::destroyScriptLights() {
	for(unsigned int i=0; i<scriptLights.size(); i++)
		if(scriptLights[i])
			mSceneMgr->destroyLight(scriptLights[i]);

	scriptLights.clear();
}


// Execute a single string of script
bool ScriptSystem::executeString(const char *str) {
	assert(mLuaVM);
	if(luaL_dostring(mLuaVM, str))
		return false;
	return true;
} 



// Load and execute a script
bool ScriptSystem::executeScript(const char *file) {
	assert(mLuaVM);

	// Find the script
	//String fil = "scripts/" + String(file);
	String fil = String(file);
	ScriptLocator *slo = static_cast<ScriptLocator*>(ResourceGroupManager::getSingletonPtr());
	Archive *arch = slo->findScript(fil);
	if(!arch) {
		OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, "ScriptSystem error: Could not find " + fil, "ScriptSystem::executeScript()");
		return false;
	}
	
	// Load it as a string
	DataStreamPtr dat = arch->open(String(file));
	if(dat.isNull()) {
		OGRE_EXCEPT(Exception::ERR_FILE_NOT_FOUND, "ScriptSystem error: Could not find " + fil, "ScriptSystem::executeScript()");
		return false;
	}

	String script = dat->getAsString();
	//if(luaL_loadfile(mLuaVM, fil.c_str())) {
	if(luaL_loadstring(mLuaVM, script.c_str())) {
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "ScriptSystem error: Unable to execute script " + fil, "ScriptSystem::executeScript()");
		return false;
	}

	// Execute
	if(lua_pcall(mLuaVM, 0, 0, 0)) {
		OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "ScriptSystem error: Error while running script " + fil + "':\n" + String(lua_tostring(mLuaVM, -1)), "ScriptSystem::executeScript()");
		return false;
	}

	return true;
}



ScriptSystem *ScriptSystem::getSingletonPtr() {
	return msSingleton;
}

ScriptSystem &ScriptSystem::getSingleton() {
	assert(msSingleton);
	return *msSingleton;
}


