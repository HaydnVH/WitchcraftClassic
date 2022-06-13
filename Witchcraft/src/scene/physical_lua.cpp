#include "physical.h"

#include "scripting/luasystem.h"

#include "physics/layers.h"
#include "physics/collider.h"

using namespace vmath;
using namespace std;

namespace {

ColliderComponent* colliders = nullptr;
RigidBodyComponent* rigidbodies = nullptr;

int lua_entity_addcollider(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* collider_type = luaL_checkstring(L, 2);

	if (COLLIDER_TYPE_STR_TO_ENUM.count(collider_type) == 0)
		{ return luaL_error(L, "Invalid collider type string '%s'.", collider_type); }

	ColliderTypeEnum collider_enum = COLLIDER_TYPE_STR_TO_ENUM.at(collider_type);
	if (collider_enum < COLLIDER_SIMPLE)
	{
		vec3 args = VEC3_ZERO;
		if (lua_istable(L, 3))
		{
			lua_pushnumber(L, 1); lua_gettable(L, -2); if (lua_isnumber(L, -1)) { args.x = (float)lua_tonumber(L, -1); } lua_pop(L, 1);
			lua_pushnumber(L, 2); lua_gettable(L, -2); if (lua_isnumber(L, -1)) { args.y = (float)lua_tonumber(L, -1); } lua_pop(L, 1);
			lua_pushnumber(L, 3); lua_gettable(L, -2); if (lua_isnumber(L, -1)) { args.z = (float)lua_tonumber(L, -1); } lua_pop(L, 1);
		}
		else
		{
			if (lua_isnumber(L, 3)) { args.x = (float)lua_tonumber(L, 3); }
			if (lua_isnumber(L, 4)) { args.y = (float)lua_tonumber(L, 4); }
			if (lua_isnumber(L, 5)) { args.z = (float)lua_tonumber(L, 5); }
		}
		colliders->MakeSimpleShape(*id, { collider_enum, args });
	}
	else
		{ return luaL_error(L, "Only simple colliders can be created through script."); }

	return 0;
}

int lua_entity_makecolliderfrommodel(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	colliders->MakeColliderFromModel(*id);
	return 0;
}

int lua_entity_setcollideroffsetpos(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	vec3 args = VEC3_ZERO;

	if (lua_istable(L, 2))
	{
		vec3 args;
		lua_pushnumber(L, 1); lua_gettable(L, 2); args.x = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 2); lua_gettable(L, 2); args.y = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
		lua_pushnumber(L, 3); lua_gettable(L, 2); args.z = (float)luaL_checknumber(L, -1); lua_pop(L, 1);
	}
	else if (lua_isuserdata(L, 2))
	{
		vec3* ptr = (vec3*)luaL_checkudata(L, 2, "vec3");
		args = *ptr;
	}
	else
	{
		args.x = (float)luaL_checknumber(L, 2);
		args.y = (float)luaL_checknumber(L, 3);
		args.z = (float)luaL_checknumber(L, 4);
	}
	colliders->setPosOffset(*id, args);
	return 0;
}

int lua_entity_setcollideroffsetrot(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	quat* arg = (quat*)luaL_checkudata(L, 2, "quat");
	
	colliders->setRotOffset(*id, *arg);
	return 0;
}

// entity.addrigidbody(entity::ID id, float mass, bool kinematic, bool triggervolume, string group, tbl mask);
int lua_entity_addrigidbody(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");

	if (colliders->isValid(*id) == false)
		{ return luaL_error(L, "This entity does not have a collider.\n"); }

	if (rigidbodies->isValid(*id))
		{ return luaL_error(L, "This entity already has a rigidbody.\n"); }

	float mass = (float)luaL_checknumber(L, 2);
	bool kinematic = (lua_toboolean(L, 3) != 0);
	bool triggervolume = (lua_toboolean(L, 4) != 0);

	const char* groupstr = luaL_checkstring(L, 5);
	int group = btBroadphaseProxy::DefaultFilter;
	if (PHYSICS_LAYER_MAP.count(groupstr) == 0)
		{ plog::warning("Invalid physics layer string '%s'.\n", groupstr); }
	else
		{ group = PHYSICS_LAYER_MAP.at(groupstr); }

	int mask = btBroadphaseProxy::AllFilter;
	if (lua_istable(L, 6))
	{
		mask = PHYSICSLAYER_DEFAULT;
		int i = 1;
		while (true)
		{
			// Get the i'th element from the table.
			lua_pushnumber(L, (lua_Number)i); lua_gettable(L, 6);

			// If the top of the stack right now is nil, then we're out of table so let's break.
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

	rigidbodies->Create(*id, mass, kinematic, triggervolume, group, mask);
	return 0;
}

int lua_entity_removephysics(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	rigidbodies->Delete(*id);
	return 0;
}

} // namespace <anon>

void PhysicalInitLua(ColliderComponent& cc, RigidBodyComponent& rb)
{
	colliders = &cc;
	rigidbodies = &rb;

	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity"); // Get the metatable for "entity" so we can put more stuff in it.
	 lua_pushcfunction(L, lua_entity_addcollider); lua_setfield(L, -2, "add_collider");
	 lua_pushcfunction(L, lua_entity_makecolliderfrommodel); lua_setfield(L, -2, "make_collider_from_model");
	 lua_pushcfunction(L, lua_entity_setcollideroffsetpos); lua_setfield(L, -2, "set_collider_offset_position");
	 lua_pushcfunction(L, lua_entity_setcollideroffsetrot); lua_setfield(L, -2, "set_collider_offset_rotation");
	 lua_pushcfunction(L, lua_entity_addrigidbody); lua_setfield(L, -2, "add_rigidbody");
	 lua_pushcfunction(L, lua_entity_removephysics); lua_setfield(L, -2, "remove_physics");
	lua_pop(L, 1);
}