#include "luasystem.h"

#include <vector>
#include <string>
using namespace std;

#include "filesystem/file_manager.h"

#include "tools/stringhelper.h"

#include "sys/printlog.h"

namespace {

constexpr const char* SCRIPT_PATH = "scripts/";
constexpr const char* SCRIPT_EXT = ".lua";
		
lua_State* L = nullptr;

/* Puts the given table on top of the stack, and returns the number of pushes which got us there. */
int GetToTable(const vector<string>& names, bool cancreate, bool skiplast)
{
	int pushes = 0;
	lua_getglobal(L, "_G"); pushes++; // push
	size_t depth = names.size();
	if (skiplast) depth--;
	for (size_t i = 0; i < depth; ++i)
	{
		// Traverse the list of names and for each name (maybe except the last) go into that table.
		lua_getfield(L, -1, names[i].c_str()); pushes++; // push
		if (lua_isnil(L, -1))
		{
			if (!cancreate)
			{
				lua_pop(L, pushes);
				return -1;
			}

			lua_pop(L, 1); // pop nil
						   // If the table we're looking for doesn't exist, create it.
			lua_newtable(L); // push
			lua_setfield(L, -2, names[i].c_str()); // pop
			lua_getfield(L, -1, names[i].c_str()); // push
		}
	}

	return pushes;
}

const char* SANDBOX_LUA_SRC =
#include "sandbox.lua"
;

const char* EVENTS_LUA_SRC =
#include "events.lua"
;

} // namespace <anon>

namespace lua {

bool Init()
{
	// Check for preconditions.
	if (L != nullptr)
	{
		plog::fatal("Programmer Error: Attempted to intialize Lua more than once.");
		return false;
	}

	if (!filemanager::isInitialized())
	{
		plog::fatal("Programmer Error: lua must initialize after filesystem.");
		return false;
	}

	L = luaL_newstate();
	luaL_openlibs(L);

	// Here we load any functions that we want our scripts to access,
	// as well as run any setup scripts that the engine relies on (including sandbox.lua).
	if (luaL_loadstring(L, SANDBOX_LUA_SRC) || lua_pcall(L, 0, 0, 0))
	{
		plog::error("Error running sandbox.lua:\n");
		plog::errmore("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error
		return false;
	}

	if (luaL_loadstring(L, EVENTS_LUA_SRC) || lua_pcall(L, 0, 0, 0))
	{
		plog::error("Error running events.lua:\n");
		plog::errmore("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error
		return false;
	}

	setFunction("SANDBOX.print", [](lua_State* L) {
		int argc = lua_gettop(L);
		stringstream ss;
		for (int i = 1; i <= argc; ++i)
		{
			const char* str = lua_tostring(L, i);
			if (str)
				plog::print(str);
		}
		plog::print("\n");
		return 0;
	});

	setFunction("SANDBOX.dofile", [](lua_State* L) {
		const char* filename = luaL_checkstring(L, 1);
		bool result = DoFile(filename);
		lua_pushboolean(L, result);
		return 1;
	});

	return true;
}

lua_State* getLuaState()
	{ return L; }

bool RunString(const char* str, const char* env)
{
	if (luaL_loadstring(L, str))
	{
		plog::error("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	if (env)
	{
		int pushes = GetToTable(splitstr(env, '.'), false, false);
		if (pushes == -1)
		{
			plog::error("Attempting to run Lua string in an environment which does not exist.\n");
			plog::errmore("Requested environment: '%s'.\n", env);
			lua_pop(L, 1);
			return false;
		}
		
		lua_setfenv(L, -(pushes + 1));
		lua_pop(L, pushes);
	}

	if (lua_pcall(L, 0, 0, 0))
	{
		plog::error("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // Pop the error mesage off the stack.
		return false;
	}

	return true;
}

bool DoFile(const char* filename)
{
	char path[44];
	snprintf(path, 44, "%s%s%s", SCRIPT_PATH, filename, SCRIPT_EXT);

	InFile file = filemanager::LoadSingleFile(path);
	if (file.is_open() == false)
	{
		plog::error("Script '%s' does not exist.\n", filename);
		return false;
	}

	string contents = file.contents();

	if (luaL_loadstring(L, contents.c_str()))
	{
		plog::error("Error loading script '%s':\n", filename);
		plog::errmore("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error
		return false;
	}
	// Right now, the only thing on the stack is a function which executes the script in its entirety.

	// Set up the protected environment.
	lua_getglobal(L, "setup_script_env");
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 0, 0))
	{
		plog::error("Error setting up environment for script '%s':\n", filename);
		plog::errmore("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error
		return false;
	}
	
	lua_getglobal(L, "SCRIPT_ENV");
	lua_getfield(L, -1, filename); // SCRIPT_ENV[filename] is the environment used by the script.

	lua_setfenv(L, -3); // pops [filename] and assigns it as the environment for "-3" (the script code)
	lua_pop(L, 1); // pops SCRIPT_ENV

	if (lua_pcall(L, 0, 0, 0))
	{
		plog::error("Error running script '%s':\n", filename);
		plog::errmore("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1); // pop error
		return false;
	}

	return true;
}

void setValue(const char* path, bool val)
{
	// Break down the table path and get to the table we want.
	vector<string> names = splitstr(string(path), '.');
	int pushes = GetToTable(names, true, true);

	// The last name in the list is the name of the value we're setting.
	lua_pushboolean(L, val);
	lua_setfield(L, -2, names.back().c_str());

	// Clean up after GetToTable()
	lua_pop(L, pushes);
}

void setValue(const char* path, int val)
{
	// Break down the table path and get to the table we want.
	vector<string> names = splitstr(string(path), '.');
	int pushes = GetToTable(names, true, true);

	// The last name in the list is the name of the value we're setting.
	lua_pushnumber(L, (lua_Number)val);
	lua_setfield(L, -2, names.back().c_str());

	// Clean up after GetToTable()
	lua_pop(L, pushes);
}

void setValue(const char* path, double val)
{
	// Break down the table path and get to the table we want.
	vector<string> names = splitstr(string(path), '.');
	int pushes = GetToTable(names, true, true);

	// The last name in the list is the name of the value we're setting.
	lua_pushnumber(L, (lua_Number)val);
	lua_setfield(L, -2, names.back().c_str());

	// Clean up after GetToTable()
	lua_pop(L, pushes);
}

void setValue(const char* path, const char* val)
{
	// Break down the table path and get to the table we want.
	vector<string> names = splitstr(string(path), '.');
	int pushes = GetToTable(names, true, true);

	// The last name in the list is the name of the value we're setting.
	lua_pushstring(L, val);
	lua_setfield(L, -2, names.back().c_str());

	// Clean up after GetToTable()
	lua_pop(L, pushes);
}

void setFunction(const char* name, lua_CFunction func)
{
	// Break down the table path and get to the table we want.
	vector<string> names = splitstr(string(name), '.');
	int pushes = GetToTable(names, true, true);

	// The last name in the list is the name of the function we're registering.
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, names.back().c_str());

	// Clean up after GetToTable()
	lua_pop(L, pushes);
}


} // namespace lua