#include "input.h"

//#include "config.h"
#include "tools/xmlhelper.h"
using namespace pugi;

#include "scripting/luasystem.h"

#include <string>
#include <map>
using namespace std;

#include "math/vmath.h"
using namespace vmath;


namespace {

map<string, uint32_t> key_map =
{
	{ "a", KEY_A },
	{ "A", KEY_A },
	{ "b", KEY_B },
	{ "B", KEY_B },
	{ "c", KEY_C },
	{ "C", KEY_C },
	{ "d", KEY_D },
	{ "D", KEY_D },
	{ "e", KEY_E },
	{ "E", KEY_E },
	{ "f", KEY_F },
	{ "F", KEY_F },
	{ "g", KEY_G },
	{ "G", KEY_G },
	{ "h", KEY_H },
	{ "H", KEY_H },
	{ "i", KEY_I },
	{ "I", KEY_I },
	{ "j", KEY_J },
	{ "J", KEY_J },
	{ "k", KEY_K },
	{ "K", KEY_K },
	{ "l", KEY_L },
	{ "L", KEY_L },
	{ "m", KEY_M },
	{ "M", KEY_M },
	{ "n", KEY_N },
	{ "N", KEY_N },
	{ "o", KEY_O },
	{ "O", KEY_O },
	{ "p", KEY_P },
	{ "P", KEY_P },
	{ "q", KEY_Q },
	{ "Q", KEY_Q },
	{ "r", KEY_R },
	{ "R", KEY_R },
	{ "s", KEY_S },
	{ "S", KEY_S },
	{ "t", KEY_T },
	{ "T", KEY_T },
	{ "u", KEY_U },
	{ "U", KEY_U },
	{ "v", KEY_V },
	{ "V", KEY_V },
	{ "w", KEY_W },
	{ "W", KEY_W },
	{ "x", KEY_X },
	{ "X", KEY_X },
	{ "y", KEY_Y },
	{ "Y", KEY_Y },
	{ "z", KEY_Z },
	{ "Z", KEY_Z },

	{ "1", KEY_1 },
	{ "2", KEY_2 },
	{ "3", KEY_3 },
	{ "4", KEY_4 },
	{ "5", KEY_5 },
	{ "6", KEY_6 },
	{ "7", KEY_7 },
	{ "8", KEY_8 },
	{ "9", KEY_9 },
	{ "0", KEY_0 },

	{ "return", KEY_ENTER },
	{ "enter", KEY_ENTER },
	{ "escape", KEY_ESC },
	{ "esc", KEY_ESC },
	{ "backspace", KEY_BACKSPACE },
	{ "tab", KEY_TAB },

	{ "space", KEY_SPACE },
	{ " ", KEY_SPACE },
	{ "minus", KEY_MINUS },
	{ "-", KEY_MINUS },
	{ "equals", KEY_EQUALS },
	{ "=", KEY_EQUALS },
	{ "left bracket", KEY_LBRACKET },
	{ "[", KEY_LBRACKET },
	{ "right bracket", KEY_RBRACKET },
	{ "]", KEY_RBRACKET },
	{ "backslash", KEY_BACKSLASH },
	{ "\\", KEY_BACKSLASH },
	{ "semicolon", KEY_SEMICOLON },
	{ ";", KEY_SEMICOLON },
	{ "apostrophe", KEY_APOSTROPHE },
	{ "'", KEY_APOSTROPHE },
	{ "grave", KEY_GRAVE },
	{ "tilde", KEY_GRAVE },
	{ "`", KEY_GRAVE },
	{ "~", KEY_GRAVE },
	{ "comma", KEY_COMMA },
	{ ",", KEY_COMMA },
	{ "period", KEY_PERIOD },
	{ ".", KEY_PERIOD },
	{ "slash", KEY_SLASH },
	{ "/", KEY_SLASH },

	{ "capslock", KEY_CAPSLOCK },

	{ "right", KEY_RIGHT },
	{ "left", KEY_LEFT },
	{ "down", KEY_DOWN },
	{ "up", KEY_UP },

	{ "shift", KEY_SHIFT },
	{ "ctrl", KEY_CONTROL },
	{ "alt", KEY_ALT },
	{ "left shift", KEY_LSHIFT },
	{ "left ctrl", KEY_LCONTROL },
	{ "left alt", KEY_LALT },
	{ "right shift", KEY_RSHIFT },
	{ "right ctrl", KEY_RCONTROL },
	{ "right alt", KEY_RALT },

	{ "f1", KEY_F1 },
	{ "f2", KEY_F2 },
	{ "f3", KEY_F3 },
	{ "f4", KEY_F4 },
	{ "f5", KEY_F5 },
	{ "f6", KEY_F6 },
	{ "f7", KEY_F7 },
	{ "f8", KEY_F8 },
	{ "f9", KEY_F9 },
	{ "f10", KEY_F10 },
	{ "f11", KEY_F11 },
	{ "f12", KEY_F12 },
};

map<string, uint32_t> mouse_button_map =
{
	{ "left", MOUSE_BUTTON_LEFT },
	{ "right", MOUSE_BUTTON_RIGHT },
	{ "middle", MOUSE_BUTTON_MIDDLE },
};

map<string, uint32_t> gamepad_button_map =
{
	{ "a", GAMEPAD_BUTTON_A },
	{ "b", GAMEPAD_BUTTON_B },
	{ "x", GAMEPAD_BUTTON_X },
	{ "y", GAMEPAD_BUTTON_Y },
	{ "back", GAMEPAD_BUTTON_BACK },
	{ "select", GAMEPAD_BUTTON_BACK },
	{ "start", GAMEPAD_BUTTON_START },
	{ "left stick", GAMEPAD_BUTTON_LEFT_THUMB },
	{ "left thumb", GAMEPAD_BUTTON_LEFT_THUMB },
	{ "lstick", GAMEPAD_BUTTON_LEFT_THUMB },
	{ "lthumb", GAMEPAD_BUTTON_LEFT_THUMB },
	{ "right stick", GAMEPAD_BUTTON_RIGHT_THUMB },
	{ "right thumb", GAMEPAD_BUTTON_RIGHT_THUMB },
	{ "rstick", GAMEPAD_BUTTON_RIGHT_THUMB },
	{ "rthumb", GAMEPAD_BUTTON_RIGHT_THUMB },
	{ "left shoulder", GAMEPAD_BUTTON_LEFT_SHOULDER },
	{ "left bumper", GAMEPAD_BUTTON_LEFT_SHOULDER },
	{ "lshoulder", GAMEPAD_BUTTON_LEFT_SHOULDER },
	{ "lbumper", GAMEPAD_BUTTON_LEFT_SHOULDER },
	{ "right shoulder", GAMEPAD_BUTTON_RIGHT_SHOULDER },
	{ "right bumper", GAMEPAD_BUTTON_RIGHT_SHOULDER },
	{ "rshoulder", GAMEPAD_BUTTON_RIGHT_SHOULDER },
	{ "rbumper", GAMEPAD_BUTTON_RIGHT_SHOULDER },
	{ "dpad up", GAMEPAD_BUTTON_DPAD_UP },
	{ "dpad down", GAMEPAD_BUTTON_DPAD_DOWN },
	{ "dpad left", GAMEPAD_BUTTON_DPAD_LEFT },
	{ "dpad right", GAMEPAD_BUTTON_DPAD_RIGHT }
};

map<string, InputControlEnum> controls_map =
{
	{ "movement", CONTROL_MOVEMENT_2D },
	{ "camera", CONTROL_CAMERA_2D },
	{ "jump", CONTROL_BUTTON_JUMP },
	{ "interact", CONTROL_BUTTON_INTERACT },
	{ "attack 1", CONTROL_BUTTON_ATTACK1 },
	{ "attack 2", CONTROL_BUTTON_ATTACK2 },
	{ "defend", CONTROL_BUTTON_DEFEND },
	{ "sprint", CONTROL_BUTTON_SPRINT },
	{ "open menu", CONTROL_BUTTON_OPENMENU },
	{ "first person", CONTROL_BUTTON_FIRSTPERSON },
	{ "crouch", CONTROL_BUTTON_CROUCH },
	{ "menu accept", CONTROL_BUTTON_MENUACCEPT },
	{ "menu back", CONTROL_BUTTON_MENUBACK },

	{ "movement posy", CONTROL_MOVEMENT_POSY },
	{ "movement negx", CONTROL_MOVEMENT_NEGX },
	{ "movement negy", CONTROL_MOVEMENT_NEGY },
	{ "movement posx", CONTROL_MOVEMENT_POSX },
	{ "camera posy", CONTROL_CAMERA_POSY },
	{ "camera negx", CONTROL_CAMERA_NEGX },
	{ "camera negy", CONTROL_CAMERA_NEGY },
	{ "camera posx", CONTROL_CAMERA_POSX },
};


struct ControlConfig
{
	uint32_t keyboard_mapping[KEYBOARD_MAX_KEYS];
	uint32_t gamepad_mapping[14];

	InputControlEnum mouse_motion_mapping;
	InputControlEnum left_thumb_mapping;
	InputControlEnum right_thumb_mapping;
	uint32_t left_trigger_mapping;
	uint32_t right_trigger_mapping;

	float left_thumb_deadzone, right_thumb_deadzone;
	float mouse_x_sensitivity, mouse_y_sensitivity;
	float kbcam_x_sensitivity, kbcam_y_sensitivity;
	bool left_thumb_deadzone_radial, right_thumb_deadzone_radial;

	bool loaded;
};

ControlConfig default_controls = {};
ControlConfig control_config = {};

constexpr const float DEFAULT_LTHUMB_DEADZONE = 7849.0f / (float)SHRT_MAX;
constexpr const float DEFAULT_RTHUMB_DEADZONE = 8689.0f / (float)SHRT_MAX;
constexpr const float DEFAULT_MOUSE_XSENSITIVITY = 1.0f;
constexpr const float DEFAULT_MOUSE_YSENSITIVITY = 1.0f;
constexpr const float DEFAULT_KBCAM_XSENSITIVITY = 1.0f;
constexpr const float DEFAULT_KBCAM_YSENSITIVITY = 1.0f;

InputState* prev_frame_state = NULL;
InputState* this_frame_state = NULL;

int lua_key_is_down(lua_State* L)
{
	const char* keyname = luaL_checkstring(L, 1);
	if (key_map.count(keyname) == 0)
	{
		return luaL_error(L, "Invalid key.");
	}

	int which_key = key_map[keyname];
	if (this_frame_state->keyboard[which_key] == true)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_key_on_down(lua_State* L)
{
	const char* keyname = luaL_checkstring(L, 1);
	if (key_map.count(keyname) == 0)
	{
		return luaL_error(L, "Invalid key.");
	}

	int which_key = key_map[keyname];
	if ((this_frame_state->keyboard[which_key] == true) && (prev_frame_state->keyboard[which_key] == false))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_key_on_up(lua_State* L)
{
	const char* keyname = luaL_checkstring(L, 1);
	if (key_map.count(keyname) == 0)
	{
		return luaL_error(L, "Invalid key.");
	}

	int which_key = key_map[keyname];
	if ((this_frame_state->keyboard[which_key] == false) && (prev_frame_state->keyboard[which_key] == true))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_mouse_is_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (mouse_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = mouse_button_map[button_name];
	if (this_frame_state->mouse.buttons & which_button)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_mouse_on_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (mouse_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = mouse_button_map[button_name];
	if ((this_frame_state->mouse.buttons & which_button) && !(prev_frame_state->mouse.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_mouse_on_up(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (mouse_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = mouse_button_map[button_name];
	if (!(this_frame_state->mouse.buttons & which_button) && (prev_frame_state->mouse.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_mouse_position(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->mouse.x;
	result->y = (float)this_frame_state->mouse.y;

	return 1;
}

int lua_mouse_movement(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->mouse.delta_x;
	result->y = (float)this_frame_state->mouse.delta_y;

	return 1;
}

int lua_mouse_wheel(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->mouse.wheel_x;
	result->y = (float)this_frame_state->mouse.wheel_y;

	return 1;
}

int lua_gamepad_is_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (gamepad_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = gamepad_button_map[button_name];
	if (this_frame_state->gamepad.buttons & which_button)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_gamepad_on_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (gamepad_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = gamepad_button_map[button_name];
	if ((this_frame_state->gamepad.buttons & which_button) && !(prev_frame_state->gamepad.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_gamepad_on_up(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (gamepad_button_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = gamepad_button_map[button_name];
	if (!(this_frame_state->gamepad.buttons & which_button) && (prev_frame_state->gamepad.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_gamepad_lthumb(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->gamepad.lthumb.x;
	result->y = (float)this_frame_state->gamepad.lthumb.y;

	return 1;
}

int lua_gamepad_rthumb(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->gamepad.rthumb.x;
	result->y = (float)this_frame_state->gamepad.rthumb.y;

	return 1;
}

int lua_gamepad_triggers(lua_State* L)
{
	vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
	luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

	result->x = (float)this_frame_state->gamepad.ltrigger;
	result->y = (float)this_frame_state->gamepad.rtrigger;

	return 1;
}

int lua_controls_is_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (controls_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = controls_map[button_name];
	if (which_button < 0 || which_button >(1 << 15))
	{
		return luaL_error(L, "Invalid button.");
	}

	if (this_frame_state->controls.buttons & which_button)
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_controls_on_down(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (controls_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = controls_map[button_name];
	if (which_button < 0 || which_button >(1 << 15))
	{
		return luaL_error(L, "Invalid button.");
	}

	if ((this_frame_state->controls.buttons & which_button) && !(prev_frame_state->controls.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_controls_on_up(lua_State* L)
{
	const char* button_name = luaL_checkstring(L, 1);
	if (controls_map.count(button_name) == 0)
	{
		return luaL_error(L, "Invalid button.");
	}

	int which_button = controls_map[button_name];
	if (which_button < 0 || which_button >(1 << 15))
	{
		return luaL_error(L, "Invalid button.");
	}

	if (!(this_frame_state->controls.buttons & which_button) && (prev_frame_state->controls.buttons & which_button))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);

	return 1;
}

int lua_controls_get(lua_State* L)
{
	const char* control_name = luaL_checkstring(L, 1);
	if (controls_map.count(control_name) == 0)
	{
		return luaL_error(L, "Invalid control.");
	}

	int which_control = controls_map[control_name];
	if (which_control == CONTROL_MOVEMENT_2D)
	{
		vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
		luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

		result->x = (float)this_frame_state->controls.movement.x;
		result->y = (float)this_frame_state->controls.movement.y;
	}
	else if (which_control == CONTROL_CAMERA_2D)
	{
		vec2* result = (vec2*)lua_newuserdata(L, sizeof(vec2));
		luaL_getmetatable(L, "vec2"); lua_setmetatable(L, -2);

		result->x = (float)this_frame_state->controls.camera.x;
		result->y = (float)this_frame_state->controls.camera.y;
	}
	else if (which_control > (1 << 15))
	{
		return luaL_error(L, "Invalid control.");
	}
	else
	{
		if (this_frame_state->controls.buttons & which_control)
			lua_pushboolean(L, true);
		else
			lua_pushboolean(L, false);
	}

	return 1;
}

} // namespace <anon>


void input::Init()
{
	// Define default controls
	default_controls.keyboard_mapping[KEY_W] = CONTROL_MOVEMENT_POSY;
	default_controls.keyboard_mapping[KEY_A] = CONTROL_MOVEMENT_NEGX;
	default_controls.keyboard_mapping[KEY_S] = CONTROL_MOVEMENT_NEGY;
	default_controls.keyboard_mapping[KEY_D] = CONTROL_MOVEMENT_POSX;
	default_controls.keyboard_mapping[KEY_UP] = CONTROL_MOVEMENT_POSY;
	default_controls.keyboard_mapping[KEY_LEFT] = CONTROL_MOVEMENT_NEGX;
	default_controls.keyboard_mapping[KEY_DOWN] = CONTROL_MOVEMENT_NEGY;
	default_controls.keyboard_mapping[KEY_RIGHT] = CONTROL_MOVEMENT_POSX;
	default_controls.keyboard_mapping[KEY_SPACE] = CONTROL_BUTTON_MENUACCEPT | CONTROL_BUTTON_INTERACT;
//	default_controls.keyboard_mapping[KEY_SPACE] = CONTROL_BUTTON_INTERACT;
//	default_controls.mouse_motion_mapping = CONTROL_CAMERA_2D;
	default_controls.left_thumb_mapping = CONTROL_MOVEMENT_2D;
	default_controls.right_thumb_mapping = CONTROL_CAMERA_2D;
	default_controls.left_thumb_deadzone = 0.2395f;
	default_controls.right_thumb_deadzone = 0.26517f;
	default_controls.left_thumb_deadzone_radial = true;
	default_controls.mouse_x_sensitivity = 0.05f;
	default_controls.mouse_y_sensitivity = 0.05f;

	control_config = default_controls;
	/*
	// Get the config section
	xml_node section = config::get_section("controls");
	xml_node node;

	// Get keyboard input
	for (node = section.child("key"); node; node = node.next_sibling("key"))
	{
		// Get the 'name' attribute, which denotes which key we're specifying, and make sure it exists.
		xml_attribute attrib = node.attribute("name");
		if (!attrib)
			continue;

		const char* key_name = attrib.value();

		// Make sure the key name is invalid.
		if (key_map.count(key_name) == 0)
			continue;

		uint32_t key_index = key_map[key_name];

		// Make sure the control name is valid.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];

		// Make sure we're actually trying to control a button here.
		if (control <= 0)
			continue;

		control_config.keyboard_mapping[key_index] |= control;
	}

	// Get keyboard-to-2d input
	for (node = section.child("keys_to_2d"); node; node = node.next_sibling("keys_to_2d"))
	{
		// Get the attributes for each direction, and make sure they all exist.
		xml_attribute posy, negx, negy, posx;
		posy = node.attribute("posy");
		negx = node.attribute("negx");
		negy = node.attribute("negy");
		posx = node.attribute("posx");
		if (!posy || !negx || !negy || !posx)
			continue;

		// Make sure they all map to real keys.
		if (key_map.count(posy.value()) == 0 ||
			key_map.count(negx.value()) == 0 ||
			key_map.count(negy.value()) == 0 ||
			key_map.count(posx.value()) == 0)
			continue;

		// Make sure we're trying to map these keys to an actual 2d control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];
		switch (control)
		{
		case CONTROL_MOVEMENT_2D:
			control_config.keyboard_mapping[key_map[posy.value()]] |= CONTROL_MOVEMENT_POSY;
			control_config.keyboard_mapping[key_map[negx.value()]] |= CONTROL_MOVEMENT_NEGX;
			control_config.keyboard_mapping[key_map[negy.value()]] |= CONTROL_MOVEMENT_NEGY;
			control_config.keyboard_mapping[key_map[posx.value()]] |= CONTROL_MOVEMENT_POSX;
			break;
		case CONTROL_CAMERA_2D:
			control_config.keyboard_mapping[key_map[posy.value()]] |= CONTROL_CAMERA_POSY;
			control_config.keyboard_mapping[key_map[negx.value()]] |= CONTROL_CAMERA_NEGX;
			control_config.keyboard_mapping[key_map[negy.value()]] |= CONTROL_CAMERA_NEGY;
			control_config.keyboard_mapping[key_map[posx.value()]] |= CONTROL_CAMERA_POSX;
			break;
		default:
			continue;
		}
	}

	// Get mouse button input
	for (node = section.child("mouse_button"); node; node = node.next_sibling("mouse_button"))
	{
		// Get the 'name' attribute, which denotes which key we're specifying, and make sure it exists.
		xml_attribute attrib = node.attribute("name");
		if (!attrib)
			continue;

		const char* button_name = attrib.value();

		// Make sure the key name is invalid.
		if (mouse_button_map.count(button_name) == 0)
			continue;

		uint32_t button_index = mouse_button_map[button_name];

		// Make sure the control name is valid.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];

		// Make sure we're actually trying to control a button here.
		if (control <= 0)
			continue;

		control_config.keyboard_mapping[button_index] |= control;
	}

	// Get mouse motion input
	for (node = section.child("mouse_motion"); node; node = node.next_sibling("mouse_motion"))
	{
		// Make sure we're trying to map a valid control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		InputControlEnum control = controls_map[node.text().as_string()];
		if (control == CONTROL_MOVEMENT_2D || control == CONTROL_CAMERA_2D)
			control_config.mouse_motion_mapping = control;
	}

	// Get gamepad thumbsticks
	for (node = section.child("gamepad_left_thumb"); node; node = node.next_sibling("gamepad_left_thumb"))
	{
		// Make sure we're trying to map a valid control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		InputControlEnum control = controls_map[node.text().as_string()];
		if (control == CONTROL_MOVEMENT_2D || control == CONTROL_CAMERA_2D)
			control_config.left_thumb_mapping = control;
	}
	for (node = section.child("gamepad_right_thumb"); node; node = node.next_sibling("gamepad_right_thumb"))
	{
		// Make sure we're trying to map a valid control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		InputControlEnum control = controls_map[node.text().as_string()];
		if (control == CONTROL_MOVEMENT_2D || control == CONTROL_CAMERA_2D)
			control_config.right_thumb_mapping = control;
	}

	// Get gamepad triggers
	for (node = section.child("gamepad_left_trigger"); node; node = node.next_sibling("gamepad_left_trigger"))
	{
		// Make sure we're trying to map a valid control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];
		if (control > 0)
			control_config.left_trigger_mapping |= control;
	}
	for (node = section.child("gamepad_right_trigger"); node; node = node.next_sibling("gamepad_right_trigger"))
	{
		// Make sure we're trying to map a valud control.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];
		if (control > 0)
			control_config.right_trigger_mapping |= control;
	}

	// Get gamepad buttons
	for (node = section.child("gamepad_button"); node; node = node.next_sibling("gamepad_button"))
	{
		// Get the 'name' attribute, which denotes which button we're specifying, and make sure it exists.
		xml_attribute attrib = node.attribute("name");
		if (!attrib)
			continue;

		const char* button_name = attrib.value();

		// Make sure the key name is invalid.
		if (gamepad_button_map.count(button_name) == 0)
			continue;

		// xinput stores gamepad buttons as bits,
		uint32_t button_bitmask = gamepad_button_map[button_name];
		uint32_t button_index = 0;
		// so we need to use a loop to figure out how to translate it back to an index.
		for (uint32_t i = 0; i < 14; ++i)
		{
			if (button_bitmask >> i == 1)
			{
				button_index = i;
				break;
			}
		}

		// Make sure the control name is valid.
		if (controls_map.count(node.text().as_string()) == 0)
			continue;

		int control = controls_map[node.text().as_string()];

		// Make sure we're actually trying to control a button here.
		if (control <= 0)
			continue;

		control_config.gamepad_mapping[button_index] |= control;
	}

	if (node = section.child("left_thumb_radial_deadzone"))
		control_config.left_thumb_deadzone_radial = true;

	if (node = section.child("right_thumb_radial_deadzone"))
		control_config.right_thumb_deadzone_radial = true;

	control_config.left_thumb_deadzone = readXML(section, "left_thumb_deadzone", DEFAULT_LTHUMB_DEADZONE);
	control_config.right_thumb_deadzone = readXML(section, "right_thumb_deadzone", DEFAULT_RTHUMB_DEADZONE);

	control_config.mouse_x_sensitivity = readXML(section, "mouse_x_sensitivity", DEFAULT_MOUSE_XSENSITIVITY);
	control_config.mouse_y_sensitivity = readXML(section, "mouse_y_sensitivity", DEFAULT_MOUSE_YSENSITIVITY);

	control_config.kbcam_x_sensitivity = readXML(section, "keyboard_camera_x_sensitivity", DEFAULT_KBCAM_XSENSITIVITY);
	control_config.kbcam_y_sensitivity = readXML(section, "keyboard_camera_y_sensitivity", DEFAULT_KBCAM_YSENSITIVITY);
	*/

	// Setup lua tables for input handling
	lua_State* L = lua::getLuaState();

	lua_newtable(L); // global "input" table.
	 lua_newtable(L); // input.key.
	  lua_pushcfunction(L, lua_key_is_down); lua_setfield(L, -2, "is_down");
	  lua_pushcfunction(L, lua_key_on_down); lua_setfield(L, -2, "on_down");
	  lua_pushcfunction(L, lua_key_on_up); lua_setfield(L, -2, "on_up");
	 lua_setfield(L, -2, "key");
	 lua_newtable(L); // input.mouse
	  lua_pushcfunction(L, lua_mouse_is_down); lua_setfield(L, -2, "is_down");
	  lua_pushcfunction(L, lua_mouse_on_down); lua_setfield(L, -2, "on_down");
	  lua_pushcfunction(L, lua_mouse_on_up); lua_setfield(L, -2, "on_up");
	  lua_pushcfunction(L, lua_mouse_position); lua_setfield(L, -2, "position");
	  lua_pushcfunction(L, lua_mouse_movement); lua_setfield(L, -2, "delta_pos");
	  lua_pushcfunction(L, lua_mouse_wheel); lua_setfield(L, -2, "delta_wheel");
	 lua_setfield(L, -2, "mouse");
	 lua_newtable(L); // input.gamepad
	  lua_pushcfunction(L, lua_gamepad_is_down); lua_setfield(L, -2, "is_down");
	  lua_pushcfunction(L, lua_gamepad_on_down); lua_setfield(L, -2, "on_down");
	  lua_pushcfunction(L, lua_gamepad_on_up); lua_setfield(L, -2, "on_up");
	  lua_pushcfunction(L, lua_gamepad_lthumb); lua_setfield(L, -2, "left_thumb");
	  lua_pushcfunction(L, lua_gamepad_rthumb); lua_setfield(L, -2, "right_thumb");
	  lua_pushcfunction(L, lua_gamepad_triggers); lua_setfield(L, -2, "triggers");
	 lua_setfield(L, -2, "gamepad");
	 lua_newtable(L); // input.controls
	  lua_pushcfunction(L, lua_controls_get); lua_setfield(L, -2, "get");
	  lua_pushcfunction(L, lua_controls_on_down); lua_setfield(L, -2, "on_down");
	  lua_pushcfunction(L, lua_controls_on_up); lua_setfield(L, -2, "on_up");
	 lua_setfield(L, -2, "controls");
	lua_setglobal(L, "INPUT");

	lua::RunString("SANDBOX.input = readonly({key = readonly(_G.INPUT.key), mouse = readonly(_G.INPUT.mouse), gamepad = readonly(_G.INPUT.gamepad), controls = readonly(_G.INPUT.controls)})", nullptr);
}

void input::Process(InputState& prev_frame_input, InputState& this_frame_input)
{
	prev_frame_state = &prev_frame_input;
	this_frame_state = &this_frame_input;

	// Do deadzone correction for thumbsticks.
	if (control_config.left_thumb_deadzone_radial)
	{
		float mag = this_frame_input.gamepad.lthumb.magnitude();
		if (mag <= control_config.left_thumb_deadzone)
		{
			this_frame_input.gamepad.lthumb = { 0, 0 };
		}
		else if (mag > 1.0f)
		{
			this_frame_input.gamepad.lthumb.normalize();
		}
	}
	else
	{
		if (fabsf(this_frame_input.gamepad.lthumb.x) <= control_config.left_thumb_deadzone)
			this_frame_input.gamepad.lthumb.x = 0;

		if (fabsf(this_frame_input.gamepad.lthumb.y) <= control_config.left_thumb_deadzone)
			this_frame_input.gamepad.lthumb.y = 0;
	}

	if (control_config.right_thumb_deadzone_radial)
	{
		float mag = this_frame_input.gamepad.rthumb.magnitude();
		if (mag <= control_config.right_thumb_deadzone)
			this_frame_input.gamepad.rthumb = { 0,0 };
		else if (mag > 1.0f)
			this_frame_input.gamepad.rthumb.normalize();
	}
	else
	{
		if (fabsf(this_frame_input.gamepad.rthumb.x) <= control_config.right_thumb_deadzone)
			this_frame_input.gamepad.rthumb.x = 0;

		if (fabsf(this_frame_input.gamepad.rthumb.y) <= control_config.right_thumb_deadzone)
			this_frame_input.gamepad.rthumb.y = 0;
	}

	// Map input to controls
	this_frame_input.controls.buttons = 0;
	this_frame_input.controls.movement = { 0, 0 };
	this_frame_input.controls.camera = { 0, 0 };

	for (uint32_t i = 0; i < KEYBOARD_MAX_KEYS; ++i)
	{
		if (this_frame_input.keyboard[i])
			this_frame_input.controls.buttons |= control_config.keyboard_mapping[i];
	}

	for (uint32_t i = 0; i < 14; ++i)
	{
		if (this_frame_input.gamepad.buttons & (1 << i))
			this_frame_input.controls.buttons |= control_config.gamepad_mapping[i];
	}

	if (this_frame_input.gamepad.ltrigger > 0.5f)
		this_frame_input.controls.buttons |= control_config.left_trigger_mapping;
	if (this_frame_input.gamepad.rtrigger > 0.5f)
		this_frame_input.controls.buttons |= control_config.right_trigger_mapping;

	vec2 mouse_motion = { (float)this_frame_input.mouse.delta_x * control_config.mouse_x_sensitivity,
		(float)this_frame_input.mouse.delta_y * control_config.mouse_y_sensitivity };

	if (control_config.mouse_motion_mapping == CONTROL_MOVEMENT_2D)
		this_frame_input.controls.movement += mouse_motion;
	else if (control_config.mouse_motion_mapping == CONTROL_CAMERA_2D)
		this_frame_input.controls.camera += mouse_motion;

	if (control_config.left_thumb_mapping == CONTROL_MOVEMENT_2D)
		this_frame_input.controls.movement += this_frame_input.gamepad.lthumb;
	else if (control_config.left_thumb_mapping == CONTROL_CAMERA_2D)
		this_frame_input.controls.camera += this_frame_input.gamepad.lthumb;

	if (control_config.right_thumb_mapping == CONTROL_MOVEMENT_2D)
		this_frame_input.controls.movement += this_frame_input.gamepad.rthumb;
	else if (control_config.right_thumb_mapping == CONTROL_CAMERA_2D)
		this_frame_input.controls.camera += this_frame_input.gamepad.rthumb;

	if (this_frame_input.controls.buttons & CONTROL_MOVEMENT_POSY)
		this_frame_input.controls.movement.y += 1;
	if (this_frame_input.controls.buttons & CONTROL_MOVEMENT_NEGX)
		this_frame_input.controls.movement.x -= 1;
	if (this_frame_input.controls.buttons & CONTROL_MOVEMENT_NEGY)
		this_frame_input.controls.movement.y -= 1;
	if (this_frame_input.controls.buttons & CONTROL_MOVEMENT_POSX)
		this_frame_input.controls.movement.x += 1;

	if (this_frame_input.controls.buttons & CONTROL_CAMERA_POSY)
		this_frame_input.controls.camera.y += 1;
	if (this_frame_input.controls.buttons & CONTROL_CAMERA_NEGX)
		this_frame_input.controls.camera.x -= 1;
	if (this_frame_input.controls.buttons & CONTROL_CAMERA_NEGY)
		this_frame_input.controls.camera.y -= 1;
	if (this_frame_input.controls.buttons & CONTROL_CAMERA_POSX)
		this_frame_input.controls.camera.x += 1;

	// Normalize movement input, so using multiple inputs doesn't let the player move faster than normal.
	if (this_frame_input.controls.movement.magnitude() > 1.0f)
		this_frame_input.controls.movement.normalize();
}