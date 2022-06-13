#include "physics_system.h"

#include "scripting/luasystem.h"
#include "layers.h"
#include "sys/printlog.h"

using namespace vmath;

namespace {

// debug.drawline(vec3 from, vec3 to, vec3 col1 = white, vec3 col2 = col1)
int lua_debug_drawline(lua_State* L)
{
	vec3 from, to, col1, col2;

	if (lua_istable(L, 1))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 1); from.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 1); from.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 1); from.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 1))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 1, "vec3");
		from = *ptr;
	}
	else
	{
		return luaL_error(L, "First argument to drawline() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 1)) );
	}

	if (lua_istable(L, 2))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 2); to.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 2); to.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 2); to.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 2, "vec3");
		to = *ptr;
	}
	else
	{
		return luaL_error(L, "Second argument to drawline() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 2)));
	}

	if (lua_istable(L, 3))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 3); col1.r = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 3); col1.g = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 3); col1.b = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else
		col1 = { 1.0, 1.0, 1.0 };

	if (lua_istable(L, 4))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 4); col2.r = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 4); col2.g = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 4); col2.b = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else
		col2 = col1;

	physics::getDebug().drawLine({ from.x, from.y, from.z }, { to.x, to.y, to.z }, { col1.r, col1.g, col1.b }, { col2.r, col2.g, col2.b });
	return 0;
}

// debug.drawsphere(vec3 pos, float radius, vec3 color
int lua_debug_drawsphere(lua_State* L)
{
	vec3 position = VEC3_ZERO;
	float radius = 0.0f;
	vec3 color = VEC3_WHITE;

	if (lua_isuserdata(L, 1))
	{
		vec3* arg = (vec3*)luaL_checkudata(L, 1, "vec3");
		position = *arg;
	}
	else if lua_istable(L, 1)
	{
		lua_pushnumber(L, 1); lua_gettable(L, 1); position.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 1); position.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 1); position.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else
		return luaL_error(L, "First argument to drawsphere() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 1)));

	radius = (float)luaL_checknumber(L, 2);

	if (lua_istable(L, 3))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 3); color.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 3); color.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 3); color.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}

	physics::getDebug().drawSphere({position.x, position.y, position.z}, radius, {color.r, color.g, color.b});
	return 0;
}

// physics.raycast(vec3 from, vec3 to, tbl mask)
int lua_physics_raycast(lua_State* L)
{
	vec3 from, to;
	int mask = PHYSICSLAYER_DEFAULT;

		if (lua_istable(L, 1))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 1); from.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 1); from.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 1); from.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 1))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 1, "vec3");
		from = *ptr;
	}
	else
		{ return luaL_error(L, "First argument to raycast() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 1))); }

	if (lua_istable(L, 2))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 2); to.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 2); to.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 2); to.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 2, "vec3");
		to = *ptr;
	}
	else
		{ return luaL_error(L, "Second argument to raycast() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 2))); }

	if (lua_istable(L, 3))
	{
		int i = 1;
		while (true)
		{
			lua_pushnumber(L, (lua_Number)i); lua_gettable(L, 3);
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				break;
			}

			const char* layerstr = luaL_checkstring(L, -1);
			if (PHYSICS_LAYER_MAP.count(layerstr) == 0)
				{ plog::warning("Invalid physics layer string '%s'.\n", layerstr); }
			else
				{ mask |= PHYSICS_LAYER_MAP.at(layerstr); }

			lua_pop(L, 1);
			++i;
		}
	}

	RaycastResult result = physics::RaycastNearest(from, to, mask);

	// Create a table to store the results of the raycast.
	lua_newtable(L);

	lua_pushboolean(L, result.hit); lua_setfield(L, -2, "hit");

	vec3* ptr = (vec3*)lua_newuserdata(L, sizeof(vec3)); luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);
	*ptr = result.normal; lua_setfield(L, -2, "normal");

	ptr = (vec3*)lua_newuserdata(L, sizeof(vec3)); luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);
	*ptr = result.position; lua_setfield(L, -2, "position");

	lua_pushnumber(L, result.sweep_dist); lua_setfield(L, -2, "sweep_dist");

	entity::ID* id = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID)); luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);
	*id = result.id; lua_setfield(L, -2, "entity");

	return 1;
}

int lua_proxy_shapecast(lua_State* L, ColliderTypeEnum collider_type, int collider_num_dimensions)
{
	vec3 from, to;
	vec3 dimensions = VEC3_ZERO;
	int mask = PHYSICSLAYER_DEFAULT;
	int maskindex = 4;

	if (lua_istable(L, 1))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 1); from.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 1); from.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 1); from.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 1))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 1, "vec3");
		from = *ptr;
	}
	else
		{ return luaL_error(L, "First argument to shapecast() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 1))); }

	if (lua_istable(L, 2))
	{
		lua_pushnumber(L, 1); lua_gettable(L, 2); to.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 2); to.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 2); to.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 2, "vec3");
		to = *ptr;
	}
	else
		{ return luaL_error(L, "Second argument to shapecast() expected a table or a vec3, got '%s'.", lua_typename(L, lua_type(L, 2))); }

	if (lua_istable(L, 3))
	{
		for (int i = 1; i <= collider_num_dimensions; ++i)
			{ lua_pushnumber(L, i); lua_gettable(L, 3); dimensions.data[i] = (float)luaL_checknumber(L, -1); lua_pop(L, 1); }
	}
	else
	{
		for (int i = 0; i < collider_num_dimensions; ++i)
			{ dimensions.data[i] = (float)luaL_checknumber(L, 3 + i); }
		maskindex = 3 + collider_num_dimensions;
	}

	// Cylinders are (radius, radius, height) instead of (radius, height)
	if (collider_type == COLLIDER_CYLINDER)
		{ dimensions.xyz = dimensions.xxy; }

	if (lua_istable(L, maskindex))
	{
		int i = 1;
		while (true)
		{
			lua_pushnumber(L, (lua_Number)i); lua_gettable(L, maskindex);
			if (lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				break;
			}

			const char* layerstr = luaL_checkstring(L, -1);
			if (PHYSICS_LAYER_MAP.count(layerstr) == 0)
				{ plog::warning("Invalid physics layer string '%s'.\n", layerstr); }
			else
				{ mask |= PHYSICS_LAYER_MAP.at(layerstr); }

			lua_pop(L, 1);
			++i;
		}
	}

	RaycastResult result = physics::ShapecastNearest(from, to, { collider_type, dimensions }, mask);

	// Create a table to store the results of the raycast.
	lua_newtable(L);

	lua_pushboolean(L, result.hit); lua_setfield(L, -2, "hit");

	vec3* ptr = (vec3*)lua_newuserdata(L, sizeof(vec3)); luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);
	*ptr = result.normal; lua_setfield(L, -2, "normal");

	ptr = (vec3*)lua_newuserdata(L, sizeof(vec3)); luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);
	*ptr = result.position; lua_setfield(L, -2, "position");

	lua_pushnumber(L, result.sweep_dist); lua_setfield(L, -2, "sweep_dist");

	entity::ID* id = (entity::ID*)lua_newuserdata(L, sizeof(entity::ID)); luaL_getmetatable(L, "entity"); lua_setmetatable(L, -2);
	*id = result.id; lua_setfield(L, -2, "entity");

	return 1;
}

// physics.spherecast(vec3 from, vec3 to, float radius, tbl layermask)
int lua_physics_spherecast(lua_State* L)
{
	return lua_proxy_shapecast(L, COLLIDER_SPHERE, 1);
}

// physics.cylindercast(vec3 from, vec3 to, float radius, float height, tbl layermask)
int lua_physics_cylindercast(lua_State* L)
{		
	return lua_proxy_shapecast(L, COLLIDER_CYLINDER, 2);
}

// physics.capsulecast(vec3 from, vec3 to, float radius, float height, tbl layermask)
int lua_physics_capsulecast(lua_State* L)
{
	return lua_proxy_shapecast(L, COLLIDER_CAPSULE, 2);
}

} // namespace <anon>

void physics::InitLua()
{
	lua_State* L = lua::getLuaState();

	// 'debug' table
	lua_newtable(L);
	 lua_pushcfunction(L, lua_debug_drawline); lua_setfield(L, -2, "drawline");
	 lua_pushcfunction(L, lua_debug_drawsphere); lua_setfield(L, -2, "drawsphere");
	lua_setglobal(L, "DEBUG");
	lua::RunString("SANDBOX.debug = readonly(_G.DEBUG)", nullptr);

	// 'physics' table
	lua_newtable(L);
	 lua_pushcfunction(L, lua_physics_raycast); lua_setfield(L, -2, "raycast");
	 lua_pushcfunction(L, lua_physics_spherecast); lua_setfield(L, -2, "spherecast");
	 lua_pushcfunction(L, lua_physics_cylindercast); lua_setfield(L, -2, "cylindercast");
	 lua_pushcfunction(L, lua_physics_capsulecast); lua_setfield(L, -2, "capsulecast");
	lua_setglobal(L, "PHYSICS");
	lua::RunString("SANDBOX.physics = readonly(_G.PHYSICS)", nullptr);
}