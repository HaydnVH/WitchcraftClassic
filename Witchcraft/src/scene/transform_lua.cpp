#include "transform.h"
#include "scripting/luasystem.h"

#include "math/vmath.h"
using namespace vmath;

namespace {

int lua_entity_add_transform(lua_State* L)
{
//	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
//	entity::transform::add(*id);
//	return 0;
	return luaL_error(L, "Adding a transform to an entity from a script is deprecated; Components should only be added during scene construction.");
}

int lua_entity_remove_transform(lua_State* L)
{
//	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
//	entity::transform::remove(*id);
//	return 0;
	return luaL_error(L, "Removing a transform from an entity from a script is deprecated; Components should only be removed during scene cleanup.");
}

// Position

int lua_transform_getpos(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

	*result = entity::transform::getPos(*id);
	return 1;
}

int lua_transform_setpos(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	if (lua_istable(L, 2))
	{
		vec3 args;
		lua_pushnumber(L, 1); lua_gettable(L, -2); args.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, -2); args.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, -2); args.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		entity::transform::setPos(*id, args);
	}
	else if (lua_isnumber(L, 2))
	{
		vec3 args;
		args.x = (float)luaL_checknumber(L, 2);
		args.y = (float)luaL_checknumber(L, 3);
		args.z = (float)luaL_checknumber(L, 4);
		entity::transform::setPos(*id, args);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* rhs = (vec3*)luaL_checkudata(L, 2, "vec3");
		entity::transform::setPos(*id, *rhs);
	}
	else
	{
		return luaL_error(L, "setpos() expects a vec3 or table containing 3 numbers.\n");
	}

	return 0;
}

int lua_transform_move(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	if (lua_istable(L, 2))
	{
		vec3 args;
		lua_pushnumber(L, 1); lua_gettable(L, -2); args.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, -2); args.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, -2); args.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		entity::transform::move(*id, args);
	}
	else if (lua_isnumber(L, 2))
	{
		vec3 args;
		args.x = (float)luaL_checknumber(L, 2);
		args.y = (float)luaL_checknumber(L, 3);
		args.z = (float)luaL_checknumber(L, 4);
		entity::transform::move(*id, args);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* rhs = (vec3*)luaL_checkudata(L, 2, "vec3");
		entity::transform::move(*id, *rhs);
	}
	else
	{
		return luaL_error(L, "move() expects a vec3 or table containing 3 numbers.\n");
	}

	return 0;
}

int lua_transform_snappos(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	entity::transform::snapPos(*id);
	return 0;
}

// Rotation

int lua_transform_getrot(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	quat* result = (quat*)lua_newuserdata(L, sizeof(quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*result = entity::transform::getRot(*id);
	return 1;

}

int lua_transform_setrot(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	quat* arg = (quat*)luaL_checkudata(L, 2, "quat");
	entity::transform::setRot(*id, *arg);
	return 0;
}

int lua_transform_rotate(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	quat* arg = (quat*)luaL_checkudata(L, 2, "quat");
	entity::transform::rotate(*id, *arg);
	return 0;
}

int lua_transform_snaprot(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	entity::transform::snapRot(*id);
	return 0;
}

int lua_transform_forward(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

	quat rot = entity::transform::getRot(*id);
	*result = rot * VEC3_FORWARD;

	return 1;
}

// Scale

int lua_transform_getscale(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	vec3* result = (vec3*)lua_newuserdata(L, sizeof(vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

	*result = entity::transform::getScale(*id);
	return 1;
}

int lua_transform_setscale(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	if (lua_istable(L, 2))
	{
		vec3 args;
		lua_pushnumber(L, 1); lua_gettable(L, -2); args.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, -2); args.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, -2); args.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		entity::transform::setScale(*id, args);
	}
	else if (lua_isnumber(L, 2))
	{
		vec3 args;
		args.x = (float)luaL_checknumber(L, 2);
		args.y = (float)luaL_checknumber(L, 3);
		args.z = (float)luaL_checknumber(L, 4);
		entity::transform::setScale(*id, args);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* rhs = (vec3*)luaL_checkudata(L, 2, "vec3");
		entity::transform::setScale(*id, *rhs);
	}
	else
	{
		return luaL_error(L, "setscale() expects a vec3 or table containing 3 numbers.\n");
	}

	return 0;
}

int lua_transform_snapscale(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (entity::transform::has(*id) == false)
		return luaL_error(L, "This entity does not have a Transform component.\n");

	entity::transform::snapScale(*id);
	return 0;
}

} // namespace <anon>

void entity::transform::InitLua()
{
	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity"); // Get the metatable for "entity" so we can put more stuff in it.
	 lua_pushcfunction(L, lua_entity_add_transform); lua_setfield(L, -2, "add_transform");
	 lua_pushcfunction(L, lua_entity_remove_transform); lua_setfield(L, -2, "remove_transform");
	 lua_pushcfunction(L, lua_transform_getpos); lua_setfield(L, -2, "getpos");
	 lua_pushcfunction(L, lua_transform_setpos); lua_setfield(L, -2, "setpos");
	 lua_pushcfunction(L, lua_transform_move); lua_setfield(L, -2, "move");
	 lua_pushcfunction(L, lua_transform_snappos); lua_setfield(L, -2, "snappos");
	 lua_pushcfunction(L, lua_transform_getrot); lua_setfield(L, -2, "getrot");
	 lua_pushcfunction(L, lua_transform_setrot); lua_setfield(L, -2, "setrot");
	 lua_pushcfunction(L, lua_transform_rotate); lua_setfield(L, -2, "rotate");
	 lua_pushcfunction(L, lua_transform_snaprot); lua_setfield(L, -2, "snaprot");
	 lua_pushcfunction(L, lua_transform_forward); lua_setfield(L, -2, "forward");
	 lua_pushcfunction(L, lua_transform_getscale); lua_setfield(L, -2, "getscale");
	 lua_pushcfunction(L, lua_transform_setscale); lua_setfield(L, -2, "setscale");
	 lua_pushcfunction(L, lua_transform_snapscale); lua_setfield(L, -2, "snapscale");
	lua_pop(L, 1);
}