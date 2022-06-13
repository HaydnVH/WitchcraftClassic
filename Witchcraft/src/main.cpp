#include "mainloop.h"

#include "sys/printlog.h"

#include "filesystem/file_manager.h"
#include "scripting/luasystem.h"
#include "sys/window.h"
#include "physics/physics_system.h"
#include "scene/scene.h"
#include "graphics/renderer.h"
#include "gui/guilayer.h"

#include "appconfig.h"

#include "sys/timer.h"

#include "math/lua/vmath_lua.h"

#include "tools/stringhelper.h"


/* A simple convenience function which shows the crash reports and then returns 1, to be returned from main(). */
int Crash()
{
	plog::ShowCrashReports();
	return 1;
}

int main(int, char**)
{
	// Hello World!

	// Initialize subsystems.
	if (!filemanager::Init()) return Crash();
	if (!lua::Init()) return Crash();
	vmath::InitLua();
	if (!window::Open()) return Crash();
	if (!physics::Init()) return Crash();
	if (!renderer::Init()) return Crash();
	scene::InitLua();

	// Timing variables used by the main loop.
	uint32_t logical_frame_counter = 0;
	uint32_t game_frame_counter = 0;
	uint32_t display_frame_counter = 0;
	double logical_time = 0.0;
	double game_time = 0.0;
	double display_time = 0.0;	

	// TODO: Fix this.  It's hideous.
	GuiLayer guilayer;
	renderer::setGuiLayer(&guilayer);

	// Load up the first level.
	scene::Load("testlevel", "default");

	// The main loop; all the magic happens here.
	{
		bool running = window::isOpen();
		double elapsed_time = 0.0;

		lua_State* L = lua::getLuaState();
		lua_newtable(L);
		lua_setglobal(L, "TIME");
		lua::RunString("SANDBOX.time = readonly(_G.TIME)", nullptr);

		sys::Timer timer;
		while (running)
		{
			timer.update();
			elapsed_time += timer.getDeltaTime();

			while (elapsed_time >= LOGICAL_SECONDS_PER_FRAME)
			{
				// Display the FPS on the title bar once per second.
				if (logical_frame_counter % LOGICAL_FRAMES_PER_SECOND == 0)
				{
					char titlestr[32];
					snprintf(titlestr, 32, "%s (FPS: %f)", app::name, 1.0 / timer.getDeltaTime());
					window::setTitle(titlestr);
				}

				running = window::Loop();

				lua_getglobal(L, "TIME");
				lua_pushnumber(L, LOGICAL_SECONDS_PER_FRAME); lua_setfield(L, -2, "delta_time");
				lua_pushnumber(L, logical_frame_counter); lua_setfield(L, -2, "frame_counter");
				lua_pushnumber(L, logical_time); lua_setfield(L, -2, "now");
				lua_pop(L, 1);

				entity::transform::Flip();
				scene::getAnimatorComponent().Flip();
				renderer::SnapFov();

				physics::getDebug().clear();

				scene::getAnimatorComponent().LogicalUpdate(LOGICAL_SECONDS_PER_FRAME);

				physics::LogicalUpdate((float)LOGICAL_SECONDS_PER_FRAME);
				scene::getRigidBodyComponent().ApplySimulatedTransforms();

				// scene.component_animator.ApplyRagdolls();

				//lua::RunString("EVENTS.logical_update:execute_global()", nullptr);
				lua_getglobal(L, "EVENTS");
				lua_getfield(L, -1, "logical_update");
				lua_getfield(L, -1, "execute_global");
				lua_pushvalue(L, -2);
				if (lua_pcall(L, 1, 0, 0))
				{
					plog::error("Error running events.logical_update:execute_global():\n");
					plog::errmore("%s\n", lua_tostring(L, -1));
					lua_pop(L, 1); // pop error
				}
				lua_pop(L, 2); // pop EVENTS.logical_update

				scene::getRigidBodyComponent().ApplyKinematicTransforms();

				guilayer.Update(window::getInputState(), LOGICAL_SECONDS_PER_FRAME);

				logical_frame_counter++;
				elapsed_time -= LOGICAL_SECONDS_PER_FRAME;
				logical_time += LOGICAL_SECONDS_PER_FRAME;
			}

			float interpolation = (float)(elapsed_time / LOGICAL_SECONDS_PER_FRAME);

			lua_getglobal(L, "TIME");
			lua_pushnumber(L, timer.getDeltaTime()); lua_setfield(L, -2, "delta_time");
			lua_pushnumber(L, display_frame_counter); lua_setfield(L, -2, "frame_counter");
			lua_pushnumber(L, display_time); lua_setfield(L, -2, "now");
			lua_pop(L, 1);

			lua_getglobal(L, "EVENTS");
			lua_getfield(L, -1, "display_update");
			lua_getfield(L, -1, "execute_global");
			lua_pushvalue(L, -2);
			lua_pushnumber(L, interpolation);
			if (lua_pcall(L, 2, 0, 0))
			{
				plog::error("Error running events.display_update:execute_global():\n");
				plog::errmore("%s\n", lua_tostring(L, -1));
				lua_pop(L, 1); // pop error
			}
			lua_pop(L, 2); // pop EVENTS.display_update


			renderer::DrawFrame(interpolation);
			display_frame_counter++;
			display_time += timer.getDeltaTime();
		}
	}

	// Shut down subsystems.
	renderer::Cleanup();
	window::Close();
	filemanager::Shutdown();

	// Display any fatal errors as message boxes.
	plog::ShowCrashReports();
	return 0;
}