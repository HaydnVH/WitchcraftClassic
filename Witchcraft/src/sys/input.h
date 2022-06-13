#ifndef HVH_WC_SYS_INPUT_H
#define HVH_WC_SYS_INPUT_H

#include <stdint.h>
#include <string>

#include "math/vmath.h"

enum InputButtonEnum
{
	MOUSE_BUTTON_LEFT = 1 << 0,
	MOUSE_BUTTON_RIGHT = 1 << 1,
	MOUSE_BUTTON_MIDDLE = 1 << 2,

	GAMEPAD_BUTTON_A = 1 << 0,
	GAMEPAD_BUTTON_B = 1 << 1,
	GAMEPAD_BUTTON_X = 1 << 2,
	GAMEPAD_BUTTON_Y = 1 << 3,
	GAMEPAD_BUTTON_START = 1 << 4,
	GAMEPAD_BUTTON_BACK = 1 << 5,
	GAMEPAD_BUTTON_LEFT_SHOULDER = 1 << 6,
	GAMEPAD_BUTTON_RIGHT_SHOULDER = 1 << 7,
	GAMEPAD_BUTTON_LEFT_THUMB = 1 << 8,
	GAMEPAD_BUTTON_RIGHT_THUMB = 1 << 9,
	GAMEPAD_BUTTON_DPAD_UP = 1 << 10,
	GAMEPAD_BUTTON_DPAD_DOWN = 1 << 11,
	GAMEPAD_BUTTON_DPAD_LEFT = 1 << 12,
	GAMEPAD_BUTTON_DPAD_RIGHT = 1 << 13,

	KEY_A = 0,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_ENTER,
	KEY_ESC,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_SPACE,
	KEY_MINUS,
	KEY_EQUALS,
	KEY_LBRACKET,
	KEY_RBRACKET,
	KEY_BACKSLASH,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_GRAVE,
	KEY_COMMA,
	KEY_PERIOD,
	KEY_SLASH,
	KEY_CAPSLOCK,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_SHIFT,
	KEY_CONTROL,
	KEY_ALT,
	KEY_LSHIFT,
	KEY_LCONTROL,
	KEY_LALT,
	KEY_RSHIFT,
	KEY_RCONTROL,
	KEY_RALT,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEYBOARD_MAX_KEYS
};


enum InputControlEnum
{
	CONTROL_NOTHING = 0,
	CONTROL_MOVEMENT_2D = -1,
	CONTROL_CAMERA_2D = -2,
	CONTROL_BUTTON_JUMP = 1 << 0,
	CONTROL_BUTTON_INTERACT = 1 << 1,
	CONTROL_BUTTON_ATTACK1 = 1 << 2,
	CONTROL_BUTTON_ATTACK2 = 1 << 3,
	CONTROL_BUTTON_DEFEND = 1 << 4,
	CONTROL_BUTTON_SPRINT = 1 << 5,
	CONTROL_BUTTON_OPENMENU = 1 << 6,
	CONTROL_BUTTON_FIRSTPERSON = 1 << 7,
	CONTROL_BUTTON_CROUCH = 1 << 8,
	CONTROL_BUTTON_MENUACCEPT = 1 << 9,
	CONTROL_BUTTON_MENUBACK = 1 << 10,

	// These aren't actual controls, they're simply used to map buttons to 2d controls.
	// They use bits 16 and above so they can be truncated when we create an input packet.
	CONTROL_MOVEMENT_POSY = 1 << 16,
	CONTROL_MOVEMENT_NEGX = 1 << 17,
	CONTROL_MOVEMENT_NEGY = 1 << 18,
	CONTROL_MOVEMENT_POSX = 1 << 19,
	CONTROL_CAMERA_POSY = 1 << 20,
	CONTROL_CAMERA_NEGX = 1 << 21,
	CONTROL_CAMERA_NEGY = 1 << 22,
	CONTROL_CAMERA_POSX = 1 << 23,
};

struct InputState
{
	InputState() { clear(); }
	InputState(const InputState&) = default;

	// Mirrors the physical state of the keyboard.
	bool keyboard[KEYBOARD_MAX_KEYS];

	// Mirrors the physical state of the mouse and its pointer.
	struct
	{
		int x, y;
		int delta_x, delta_y;
		int wheel_x, wheel_y;
		uint8_t buttons;
	} mouse;

	// Mirrors the physical state of the gamepad.  XInput is assumed.
	struct
	{
		vmath::vec2 lthumb, rthumb;
		float ltrigger, rtrigger;
		uint16_t buttons;
	} gamepad;

	// Enables input of UTF-8 text.
	std::string text;

	// Once physical input has been translated to game controls, this structure is filled in.
	// For multiplayer, this should be the only part required to send in the packet,
	// but we might be able to use int8 instead of float for move and camera to save 12 bytes.
	struct
	{
		vmath::vec2 movement, camera;
		uint32_t buttons;
	} controls;

	// Clears the structure to a default state with nothing pressed.
	void clear()
	{
		memset(&keyboard, 0, sizeof(keyboard));
		memset(&mouse, 0, sizeof(mouse));
		memset(&gamepad, 0, sizeof(gamepad));
		memset(&controls, 0, sizeof(controls));
		text.clear();
	}
};

namespace input
{
	void Init();
	void Process(InputState& prev_state, InputState& now_state);
}

#endif // HVH_SYS_INPUT_H