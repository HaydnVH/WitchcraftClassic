#include "scene.h"
#include "scripting/luasystem.h"

namespace {

int lua_entity_create(lua_State* L)
{
	entity::ID* id = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID));
	luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);

	*id = scene::SpawnEntity();
	return 1;
}

int lua_entity_destroy(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	scene::DestroyEntity(*id);
	*id = {};

	return 0;
}

int lua_entity_eq(lua_State* L)
{
	entity::ID* lhs = (entity::ID*)luaL_checkudata(L, 1, "entity");
	entity::ID* rhs = (entity::ID*)luaL_checkudata(L, 2, "entity");

	if (*lhs == *rhs)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_entity_index(lua_State* L)
{
	entity::ID* self = (entity::ID*)luaL_checkudata(L, 1, "entity");

	// Check to see if the key already exists in the entity metatable.
	// If it does, then we're trying to call a function, not access variables.
	luaL_getmetatable(L, "entity"); // push "entity:meta"
	lua_pushvalue(L, 2); // push <key>
	lua_rawget(L, -2); // pops <key>, pushes "entity:meta.<key>"
	if (!lua_isnil(L, -1)) // If "entity:meta.<key> exists...
	{
		lua_remove(L, -2); // pop "entity:meta"
		return 1; // The top of the stack is currently "entity:meta<key>", so we return it.
	}
	else
	{
		lua_pop(L, 2); // Clean up after ourselves
		return luaL_error(L, "Attempting to access field '%s' in 'entity' which does not exist.\n", lua_tostring(L, 2));
	}
}

int lua_entity_tostring(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");

	char str[40];
	if (entity::name::has(*id))
		{ snprintf(str, 40, "<%i>, '%s'", *id, entity::name::get(*id).c_str() ); }
	else
		{ snprintf(str, 40, "<%i>", *id); }

	lua_pushstring(L, str);
	return 1;
}

int lua_entity_getindex(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	lua_pushnumber(L, *id);
	return 1;
}

} // namespace <anon>

void scene::InitLua()
{
	lua_State* L = lua::getLuaState();

	luaL_newmetatable(L, "entity"); // This will be the metatable for the "entity" userdata type.
	 lua_pushcfunction(L, lua_entity_index); lua_setfield(L, -2, "__index");
	 lua_pushcfunction(L, lua_entity_eq); lua_setfield(L, -2, "__eq");
	 lua_pushcfunction(L, lua_entity_tostring); lua_setfield(L, -2, "__tostring");
	 lua_pushcfunction(L, lua_entity_destroy); lua_setfield(L, -2, "destroy");
	 lua_pushcfunction(L, lua_entity_getindex); lua_setfield(L, -2, "index");
	 lua_pushstring(L, "entity"), lua_setfield(L, -2, "typename");
	lua_pop(L, 1); // pop entity:meta

	lua_newtable(L); // 'entity' table
	 lua_pushcfunction(L, lua_entity_create); lua_setfield(L, -2, "create");
	lua_setglobal(L, "ENTITY");

	lua::RunString("SANDBOX.entity = readonly(_G.ENTITY)", nullptr);

	entity::transform::InitLua();
	entity::name::InitLua();
	entity::script::InitLua();
	getRenderableComponent().InitLua();
	getAnimatorComponent().InitLua();
	PhysicalInitLua(getColliderComponent(), getRigidBodyComponent());
}