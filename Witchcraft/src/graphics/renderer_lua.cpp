#include "renderer.h"
#include "scripting/luasystem.h"

namespace {

Renderer* renderer;

} // namespace <anon>

int lua_renderer_setcameraentity(lua_State* L)
{
	entity::ID* id = (entity::ID*)luaL_checkudata(L, 1, "entity");
	renderer::setCameraEntity(*id);
	return 0;
}

int lua_renderer_setznear(lua_State* L)
{
	float arg = (float)luaL_checknumber(L, 1);
	renderer::setZnear(arg);
	return 0;
}

int lua_renderer_setzfar(lua_State* L)
{
	float arg = (float)luaL_checknumber(L, 1);
	renderer::setZfar(arg);
	return 0;
}

int lua_renderer_setfov(lua_State* L)
{
	float arg = (float)luaL_checknumber(L, 1);
	renderer::setFov(arg);
	return 0;
}

int lua_renderer_snapfov(lua_State* L)
{
	renderer::SnapFov();
	return 0;
}

int lua_renderer_loadskybox(lua_State* L)
{
	renderer::LoadSkybox(luaL_checkstring(L, 1));
	return 0;
}

void renderer::InitLua()
{
	lua_State* L = lua::getLuaState();

	lua_newtable(L); // the "renderer" table
	 lua_pushcfunction(L, lua_renderer_setznear); lua_setfield(L, -2, "setznear");
	 lua_pushcfunction(L, lua_renderer_setzfar); lua_setfield(L, -2, "setzfar");
	 lua_pushcfunction(L, lua_renderer_setfov); lua_setfield(L, -2, "setfov");
	 lua_pushcfunction(L, lua_renderer_snapfov); lua_setfield(L, -2, "snapfov");
	 lua_pushcfunction(L, lua_renderer_setcameraentity); lua_setfield(L, -2, "set_camera_entity");
	 lua_pushcfunction(L, lua_renderer_loadskybox); lua_setfield(L, -2, "load_skybox");
	lua_setglobal(L, "RENDERER");

	lua::RunString("SANDBOX.renderer = readonly(_G.RENDERER)", nullptr);
}