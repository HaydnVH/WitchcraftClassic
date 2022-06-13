#include "script.h"
#include "component.h"

#include "scripting/luasystem.h"
#include "sys/printlog.h"

#include <set>
#include <unordered_map>
using namespace std;

#include "tools/fixedstring.h"

namespace entity {
namespace script {

namespace {

	unordered_multimap<ID, FixedString<32>> map;

} // namespace <anon>

void* Allocate(void* memory, size_t capacity)
{
	map.reserve(capacity);
	return memory;
}

void Clear()
{
	for (auto it = map.begin(); it != map.end(); ++it)
		{ detach(it->first, it->second); }
	map.clear();
}

void attach(ID id, FixedString<32> scriptname, rapidjson::Value::ConstMemberIterator parameters)
{
	if (has(id, scriptname))
		{ plog::error("Cannot attach script to entity; script already present.\n"); return; }

	map.insert({ id, scriptname });

	// BIG TODO: Convert 'parameters' to a lua table so they can be sent to the script.

	// Get the lua state
	lua_State* L = lua::getLuaState();

	// Load the script, if it isn't already loaded.
	lua_getglobal(L, "SCRIPT_ENV");
	lua_getfield(L, -1, scriptname.c_str);
	if (lua_isnil(L, -1))
	{
		lua::DoFile(scriptname.c_str);
		lua_pop(L, 1); // pop nil
		lua_getfield(L, -1, scriptname.c_str); // push the new environment we just created

		// If it's *still* nil, then we failed to initialize the script.
		// Cancel now before something goes horribly, horribly wrong.
		if (lua_isnil(L, -1))
		{
			plog::error("Failed to initialize script '%s'.\n", scriptname.c_str);
			return;
		}
	}

	// Right now, -2 should be SCRIPT_ENV, and -1 should be "scriptname".
	// Any functions we care about ought to be here.
	lua_getfield(L, -1, "on_init");
	if (lua_isfunction(L, -1))
	{
		entity::ID* arg = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID));
		luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);
		*arg = id;
		if (lua_pcall(L, 1, 0, 0)) // call SCRIPT_ENV.'scriptname'.on_init(id).
		{
			plog::error("Failed to run '%s.on_init(%i):\n%s\n", scriptname.c_str, entity::toString(id), lua_tostring(L, -1) );
			lua_pop(L, 1); // pop error
		}
	}
	else if (!lua_isnil(L, -1))
	{
		plog::warning("Script '%s' contains an entry for 'on_init' which is not a function.\n", scriptname.c_str);
		lua_pop(L, 1);
	}

	lua_pop(L, 2); // pop SCRIPT_ENV.'scriptname'
}

void detach(ID id, FixedString<32> scriptname)
{
	if (!has(id, scriptname))
		return;

	lua_State* L = lua::getLuaState();

	lua_getglobal(L, "SCRIPT_ENV");
	lua_getfield(L, -1, scriptname.c_str);
	if (lua_isnil(L, -1))
	{
		plog::error("Attempting to remove script '%s' from an entity:\n", scriptname.c_str);
		plog::errmore("Script does not exist!\n");
		lua_pop(L, 2);
	}

	lua_getfield(L, -1, "on_cleanup");
	if (lua_isfunction(L, -1))
	{
		entity::ID* arg = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID));
		luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);
		*arg = id;
		if (lua_pcall(L, 1, 0, 0)) // call SCRIPT_ENV.'scriptname'.on_destruct(id).
		{
			plog::error("Failed to run '%s.on_cleanup(%i):\n%s\n", scriptname.c_str, entity::toString(id), lua_tostring(L, -1));
			lua_pop(L, 1); // pop error
		}
	}
	else if (!lua_isnil(L, -1))
	{
		plog::warning("Script '%s' contains an entry for 'on_cleanup' which is not a function.\n", scriptname.c_str);
		lua_pop(L, 1);
	}
}

void detachAll(ID id)
{
	if (map.count(id) == 0)
		return;

	auto range = map.equal_range(id);
	for (auto it = range.first; it != range.second; ++it)
	{
		detach(id, it->second);
	}

	map.erase(id);
}

bool has(ID id, FixedString<32> scriptname)
{
	auto range = map.equal_range(id);
	if (range.first == map.end())
		return false;

	for (auto it = range.first; it != range.second; ++it)
	{
		if (scriptname == it->second.c_str)
			return true;
	}
	return false;
}

}} // namespace entity::script