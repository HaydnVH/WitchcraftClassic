#include "renderable.h"
#include "scripting/luasystem.h"

namespace {

RenderableComponent* rc = nullptr;

int lua_entity_addrenderable(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	rc->AddEmpty(*id);
	return 0;
}

int lua_entity_removerenderable(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	rc->Remove(*id);
	return 0;
}

int lua_entity_loadmodel(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	if (rc->hasEntry(*id) == false)
		return luaL_error(L, "This entity does not have a Renderable component.");

	rc->LoadModel(*id, luaL_checkstring(L, 2));
	return 0;
}

} // namespace <anon>

void RenderableComponent::InitLua()
{
	rc = this;
	lua_State* L = lua::getLuaState();

	luaL_getmetatable(L, "entity");
	 lua_pushcfunction(L, lua_entity_addrenderable); lua_setfield(L, -2, "add_renderable");
	 lua_pushcfunction(L, lua_entity_removerenderable); lua_setfield(L, -2, "remove_renderable");
	 lua_pushcfunction(L, lua_entity_loadmodel); lua_setfield(L, -2, "load_model");
	lua_pop(L, 1);
}