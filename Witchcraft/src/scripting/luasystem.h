#ifndef HVH_WC_SCRIPTING_LUASYSTEM_H
#define HVH_WC_SCRIPTING_LUASYSTEM_H

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace lua {

	/* Initializes the Lua state and sets up the sandbox environment. */
	/* Must initialize after: filemanager. */
	/* Returns false if initialization fails. */
	bool Init();
	/* Returns a pointer to the global lua state. */
	lua_State* getLuaState();

	/* Executes a single lua script within its own sandbox. */
	bool DoFile(const char* filename);
	/* Finds every instance of a given script known to the filesystem, and runs each of them. */
	bool DoEachFile(const char* filename);

	/* Run a string as if it were a lua script, from within the given environment. */
	bool RunString(const char* str, const char* env);

	/* Sets the variable at the given path to be equal to the given boolean. */
	void setValue(const char* path, bool val);
	/* Sets the variable at the given path to be equal to the given integer. */
	void setValue(const char* path, int val);
	/* Sets the variable at the given path to be equal to the given real number. */
	void setValue(const char* path, double val);
	/* Sets the variable at the given path to be equal to the given string. */
	void setValue(const char* path, const char* val);
	/* Sets the variable at the given path to be equal to the given C function. */
	void setFunction(const char* path, lua_CFunction func);

} // namespace lua

#endif // HVH_WC_SCRIPTING_LUASYSTEM_H