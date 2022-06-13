#include "script.h"

#include "scripting/luasystem.h"

namespace
{

int lua_entity_addscript(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* scriptname = luaL_checkstring(L, 2);

	entity::script::attach(*id, scriptname);
	return 0;
}

int lua_entity_hasscript(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* scriptname = luaL_checkstring(L, 2);

	if (entity::script::has(*id, scriptname))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_entity_removescript(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* scriptname = luaL_checkstring(L, 2);

	entity::script::detach(*id, scriptname);
	return 0;
}

} // namespace <anon>

void entity::script::InitLua()
{
	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity");
	 lua_pushcfunction(L, lua_entity_addscript); lua_setfield(L, -2, "add_script");
	 lua_pushcfunction(L, lua_entity_hasscript); lua_setfield(L, -2, "has_script");
	 lua_pushcfunction(L, lua_entity_removescript); lua_setfield(L, -2, "remove_script");
	lua_pop(L, 1);
}