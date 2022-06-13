#include "name.h"

#include "scripting/luasystem.h"

namespace {

int lua_entity_getname(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::name::has(*id) == false)
		return luaL_error(L, "This entity does not have a Name component.");

	lua_pushstring(L, entity::name::get(*id).c_str());
	return 1;
}

int lua_entity_setname(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (lua_gettop(L) == 1)
		entity::name::remove(*id);
	else
		entity::name::set(*id, luaL_checkstring(L, 2));

	return 0;
}

int lua_entity_findwithname(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	entity::ID result = entity::name::Find(name);
	if (result != 0)
	{
		entity::ID* id = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID));
		luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);
		*id = result;
		return 1;
	}
	else
	{
		lua_pushnil(L);
		return 1;
	}
}

int lua_entity_findallwithname(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	std::vector<entity::ID> results = entity::name::FindAll(name);
	if (results.size() > 0)
	{
		lua_newtable(L); // results table
		for (size_t i = 0; i < results.size(); ++i)
		{
			lua_pushnumber(L, (lua_Number)i + 1); // push key
			entity::ID* id = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID)); // create/push entity
			luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2); // assign metatable to entity
			lua_settable(L, -3); // pop key and entity, assigning to results table.
		}
		return 1;
	}
	else
	{
		lua_pushnil(L);
		return 1;
	}
}

} // namespace <anon>

void entity::name::InitLua()
{
	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity");
	 lua_pushcfunction(L, lua_entity_getname); lua_setfield(L, -2, "getname");
	 lua_pushcfunction(L, lua_entity_setname); lua_setfield(L, -2, "setname");
	lua_pop(L, 1);

	lua_getglobal(L, "ENTITY");
	 lua_pushcfunction(L, lua_entity_findwithname); lua_setfield(L, -2, "find_with_name");
	 lua_pushcfunction(L, lua_entity_findallwithname); lua_setfield(L, -2, "find_all_with_name");
	lua_pop(L, 1);
}