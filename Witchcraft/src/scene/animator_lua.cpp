#include "animator.h"
#include "scripting/luasystem.h"

namespace {

AnimatorComponent* ac = nullptr;

int lua_entity_addanimator(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	ac->AddTo(*id);
	return 0;
}

int lua_entity_removeanimator(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	ac->RemoveEntry(*id);
	return 0;
}

int lua_entity_hasanimator(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	bool result = ac->hasEntry(*id);
	lua_pushboolean(L, result);
	return 1;
}

int lua_entity_importanimations(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* arg = luaL_checkstring(L, 2);
	ac->ImportAnimations(*id, arg);
	return 0;
}

int lua_entity_playanimation(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	int animindex = -1;
	const char* animname = luaL_checkstring(L, 3);

	bool loop = false;
	if (lua_isboolean(L, 4))
		{ loop = (lua_toboolean(L, 4) != 0); }

	float speed = 1.0f;
	if (lua_isnumber(L, 5))
		{ speed = (float)lua_tonumber(L, 5); }

	float transition = 0.0f;
	if (lua_isnumber(L, 6))
		{ transition = (float)lua_tonumber(L, 6); }

	ac->PlayAnimation(*id, layer, animname, loop, speed, transition);

	return 0;
}

int lua_entity_getanimname(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	const char* animname = ac->getCurrentAnimName(*id, layer);
	lua_pushstring(L, animname);
	return 1;
}

int lua_entity_getanimspeed(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	float speed = ac->getAnimSpeed(*id);
	lua_pushnumber(L, (lua_Number)speed);
	return 1;
}

int lua_entity_setanimspeed(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	float speed = (float)luaL_checknumber(L, 2);
	ac->setAnimSpeed(*id, speed);
	return 0;
}

int lua_entity_islooping(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	bool looping = ac->isLooping(*id, layer);
	lua_pushboolean(L, looping);
	return 1;
}

int lua_entity_setlooping(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	bool looping = (lua_toboolean(L, 2) != 0);
	ac->setLooping(*id, layer, looping);
	return 0;
}

int lua_entity_getanimduration(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	float duration = ac->getAnimDuration(*id, layer);
	lua_pushnumber(L, (lua_Number)duration);
	return 1;
}

int lua_entity_getanimtime(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	float time = ac->getAnimTime(*id, layer);
	lua_pushnumber(L, (lua_Number)time);
	return 1;
}

int lua_entity_getanimtimeremaining(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	double duration = ac->getAnimDuration(*id, layer);
	double time = ac->getAnimTime(*id, layer);
	lua_pushnumber(L, (lua_Number)(duration - time));
	return 1;
}

int lua_entity_getanimatedtransform(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);

	vmath::vec3* pos = (vmath::vec3*)lua_newuserdata(L, sizeof(vmath::vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);
	*pos = vmath::VEC3_ZERO;

	vmath::quat* rot = (vmath::quat*)lua_newuserdata(L, sizeof(vmath::quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);
	*rot = vmath::QUAT_IDENTITY;

	ac->getAnimatedTransform(*id, bone_name, *pos, *rot);
	return 2;
}

int lua_entity_setadditiveposition(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);
	vmath::vec3* pos = (vmath::vec3*)luaL_checkudata(L, 3, "vec3");
	ac->setBoneAdditivePosition(*id, bone_name, *pos);
	return 0;
}

int lua_entity_setadditivepositionlocal(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);
	vmath::vec3* pos = (vmath::vec3*)luaL_checkudata(L, 3, "vec3");
	ac->setBoneAdditivePositionLocal(*id, bone_name, *pos);
	return 0;
}

int lua_entity_getadditiveposition(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);

	vmath::vec3* pos = (vmath::vec3*)lua_newuserdata(L, sizeof(vmath::vec3));
	luaL_getmetatable(L, "vec3"); lua_setmetatable(L, -2);

	*pos = ac->getBoneAdditivePosition(*id, bone_name);
	return 1;
}

int lua_entity_clearadditivepositions(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	ac->ClearAdditivePositions(*id);
	return 0;
}

int lua_entity_setadditiverotation(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);
	vmath::quat* rot = (vmath::quat*)luaL_checkudata(L, 3, "quat");
	ac->setBoneAdditiveRotation(*id, bone_name, *rot);
	return 0;
}

int lua_entity_setadditiverotationlocal(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);
	vmath::quat* rot = (vmath::quat*)luaL_checkudata(L, 3, "quat");
	ac->setBoneAdditiveRotationLocal(*id, bone_name, *rot);
	return 0;
}

int lua_entity_getadditiverotation(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	const char* bone_name = luaL_checkstring(L, 2);

	vmath::quat* rot = (vmath::quat*)lua_newuserdata(L, sizeof(vmath::quat));
	luaL_getmetatable(L, "quat"); lua_setmetatable(L, -2);

	*rot = ac->getBoneAdditiveRotation(*id, bone_name);
	return 1;
}

int lua_entity_clearadditiverotations(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	ac->ClearAdditiveRotations(*id);
	return 0;
}

int lua_entity_setlayerweight(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	float weight = (float)luaL_checknumber(L, 3);
	ac->setAnimLayerWeight(*id, layer, weight);
	return 0;
}

int lua_entity_getlayerweight(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	int layer = (int)luaL_checknumber(L, 2);
	float weight = ac->getAnimLayerWeight(*id, layer);
	lua_pushnumber(L, (lua_Number)weight);
	return 1;
}

int lua_entity_drawdebug(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	ac->DrawDebug(*id);
	return 0;
}

} // namespace <anon>

void AnimatorComponent::InitLua()
{
	ac = this;
	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity");
	 lua_pushcfunction(L, lua_entity_addanimator); lua_setfield(L, -2, "add_animator");
	 lua_pushcfunction(L, lua_entity_removeanimator); lua_setfield(L, -2, "remove_animator");
	 lua_pushcfunction(L, lua_entity_hasanimator); lua_setfield(L, -2, "has_animator");
	 lua_pushcfunction(L, lua_entity_importanimations); lua_setfield(L, -2, "import_animations");
	 lua_pushcfunction(L, lua_entity_playanimation); lua_setfield(L, -2, "play_animation");
	 lua_pushcfunction(L, lua_entity_getanimname); lua_setfield(L, -2, "get_animation_name");
	 lua_pushcfunction(L, lua_entity_getanimspeed); lua_setfield(L, -2, "get_animation_speed");
	 lua_pushcfunction(L, lua_entity_setanimspeed); lua_setfield(L, -2, "set_animation_speed");
	 lua_pushcfunction(L, lua_entity_islooping); lua_setfield(L, -2, "is_animation_looping");
	 lua_pushcfunction(L, lua_entity_setlooping); lua_setfield(L, -2, "set_animation_looping");
	 lua_pushcfunction(L, lua_entity_getanimduration); lua_setfield(L, -2, "get_animation_duration");
	 lua_pushcfunction(L, lua_entity_getanimtime); lua_setfield(L, -2, "get_animation_time");
	 lua_pushcfunction(L, lua_entity_getanimtimeremaining); lua_setfield(L, -2, "get_animation_time_remaining");
	 lua_pushcfunction(L, lua_entity_getanimatedtransform); lua_setfield(L, -2, "get_animated_transform");
	 lua_pushcfunction(L, lua_entity_setadditiveposition); lua_setfield(L, -2, "set_bone_additive_position");
	 lua_pushcfunction(L, lua_entity_setadditivepositionlocal); lua_setfield(L, -2, "set_bone_additive_position_local");
	 lua_pushcfunction(L, lua_entity_getadditiveposition); lua_setfield(L, -2, "get_bone_additive_position");
	 lua_pushcfunction(L, lua_entity_clearadditivepositions); lua_setfield(L, -2, "clear_additive_positions");
	 lua_pushcfunction(L, lua_entity_setadditiverotation); lua_setfield(L, -2, "set_bone_additive_rotation");
	 lua_pushcfunction(L, lua_entity_setadditiverotationlocal); lua_setfield(L, -2, "set_bone_additive_rotation_local");
	 lua_pushcfunction(L, lua_entity_getadditiverotation); lua_setfield(L, -2, "get_bone_additive_rotation");
	 lua_pushcfunction(L, lua_entity_clearadditiverotations); lua_setfield(L, -2, "clear_additive_rotations");
	 lua_pushcfunction(L, lua_entity_setlayerweight); lua_setfield(L, -2, "set_anim_layer_weight");
	 lua_pushcfunction(L, lua_entity_getlayerweight); lua_setfield(L, -2, "get_anim_layer_weight");
	 lua_pushcfunction(L, lua_entity_drawdebug); lua_setfield(L, -2, "draw_skeleton_debug");

	lua_pop(L, 1);
}