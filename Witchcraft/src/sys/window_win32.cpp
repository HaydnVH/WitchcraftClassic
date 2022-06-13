#ifdef PLATFORM_WIN32
#include "window.h"

#include <Windows.h>

#include "printlog.h"
#include "input.h"
#include "tools/stringhelper.h"
#include "appconfig.h"

#include <Windows.h>
#include <Xinput.h>
#include "LooplessSizeMove.h"

#include <unordered_map>
using namespace std;

namespace window {

// The Window::<anon> namespace stores all of the static data our window needs.
namespace {

bool running = false, active = false;

struct
{
	int width, height;
	int fs_width, fs_height;
	bool maximized, fullscreen, borderless_fs, vsync;
} config =
{
	1280, 720,
	0, 0,
	false, false, true, false
};

bool mouse_shown = true;
int old_mouse_x = 0, old_mouse_y = 0;
int last_shown_mouse_x = 0, last_shown_mouse_y = 0;

HWND window_handle = nullptr;
HDC display_context = nullptr;
HGLRC rendering_context = nullptr;

InputState prev_input_state = {}, current_input_state = {};
bool controller_connected = false;

// This is the window procedure as mandates by the Win32 API.
// It handles all of the messages our window receives from outside sources.
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	// 'Destroy', 'Close', and 'Quit' all have the same result: signal the program to stop running.
	case WM_DESTROY:
	case WM_CLOSE:
	case WM_QUIT:
		running = false;
		return 0;
	case WM_SIZE:
		// If the window is NOT fullscreen...
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_OVERLAPPEDWINDOW)
		{
			// User hit the 'maximize' button,
			if (wparam == SIZE_MAXIMIZED)
			{
				// So the window is Maximized and NOT fullscreen.
				config.maximized = true;
				config.fullscreen = false;
			}
			// The window is not maximized,
			else if (wparam == SIZE_RESTORED)
			{
				// So we have to get the real size of the window.
				config.maximized = false;
				getWindowSize(config.width, config.height);
			}
		}
		return 0;
	case WM_MOUSEWHEEL:
		current_input_state.mouse.wheel_y += (HIWORD(wparam) / WHEEL_DELTA);
		return 0;
	case WM_MOUSEHWHEEL:
		current_input_state.mouse.wheel_x += (HIWORD(wparam) / WHEEL_DELTA);
		return 0;
	case WM_CHAR:
	{
		// The window is receiving text input.  Win32 presents this as a single utf-16 character.
		wstring utf16_char;
		utf16_char += (wchar_t)wparam;
		// Convert this to a utf-8 sequence.
		string utf8_char = utf16_to_utf8(utf16_char);
		current_input_state.text += utf8_char;
		return 0;
	}
	case WM_INPUT:
		if (mouse_shown == false)
		{
			// This is how we receive mouse input when the mouse cursor is hidden.
			RAWINPUT input;
			uint32_t inputsize = sizeof(input);
			GetRawInputData((HRAWINPUT)lparam, RID_INPUT, &input, &inputsize, sizeof(RAWINPUTHEADER));

			if (input.header.dwType == RIM_TYPEMOUSE)
			{
				current_input_state.mouse.delta_x += input.data.mouse.lLastX;
				current_input_state.mouse.delta_y += input.data.mouse.lLastY;
			}

			// Snap the actual cursor position to the center of the window,
			// so the user can't accidentally click on something outside the window.
			if (active)
			{
				RECT screen;
				GetWindowRect(hwnd, &screen);
				int x = (screen.left + screen.right) / 2;
				int y = (screen.top + screen.bottom) / 2;
				SetCursorPos(x, y);
			}
		}
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wparam) == WA_INACTIVE) // Window is deactivated
		{
			active = false;
			if (mouse_shown == false)
			{
				ShowCursor(true);
				SetCursorPos(last_shown_mouse_x, last_shown_mouse_y);
			}
		}
		else // Window is activated
		{
			active = true;
			if (mouse_shown == false)
			{
				POINT pos;
				GetCursorPos(&pos);
				last_shown_mouse_x = pos.x;
				last_shown_mouse_y = pos.y;
				ShowCursor(false);
			}
		}
		return 0;
	}

	return LSMProc(hwnd, msg, wparam, lparam);
//	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef RENDERER_OPENGL
#include <gl/glew.h>
#include <gl/wglew.h>
#include <gl/GL.h>
bool InitGraphics()
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, // color bits
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24, // depth bits
		8, // stencil bits
		0, // aux bits
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	display_context = GetDC(window_handle);
	if (!display_context)
	{
		plog::fatal("Failed to get first window's display context.\n");
		return false;
	}

	int pf = ChoosePixelFormat(display_context, &pfd);
	if (!pf)
	{
		plog::fatal("Failed to choose an appropriate pixel format.\n");
		return false;
	}

	if (!SetPixelFormat(display_context, pf, &pfd))
	{
		plog::fatal("Failed to set the first pixel format.\n");
		return false;
	}

	rendering_context = wglCreateContext(display_context);
	if (!rendering_context)
	{
		plog::fatal("Failed to create the first OpenGL context.\n");
		return false;
	}

	if (!wglMakeCurrent(display_context, rendering_context))
	{
		plog::fatal("Failed to activate the first OpenGL context.\n");
		return false;
	}

	// Use the OpenGL context to load OpenGL extensions.
	if (glewInit() != GLEW_OK)
	{
		plog::fatal("GLEW failed to load OpenGL extensions.\n");
		return false;
	}

	// Figure out the OpenGL version supported by the driver.
	int majorver = 0, minorver = 0, glver = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorver);
	glGetIntegerv(GL_MINOR_VERSION, &minorver);
	glver = minorver + (majorver * 10);
	if (glver < 43 && glver >= 40)
	{
		plog::warning("OpenGL 4.3 or higher is highly reccomended.  Please update your graphics drivers.\n");
	}
	else if (glver < 30)
	{
		plog::fatal("OpenGL 3.0 or higher is required.  If updating your graphics drivers doesn't solve this issue, you may need to upgrade your graphics hardward.\n");
		return false;
	}

	// Delete the context we just made and destroy the window.
	wglMakeCurrent(display_context, nullptr);
	wglDeleteContext(rendering_context);
	DestroyWindow(window_handle);

	// Create a new window.  With sRGB, and multisampling!
	window_handle = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"Witchcraft",
		utf8_to_utf16(app::window_title).c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		config.width, config.height,
		nullptr, nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	if (!window_handle)
	{
		plog::fatal("Failed to open the second window: Error code %i.\n", GetLastError());
		return false;
	}

	display_context = GetDC(window_handle);
	if (!display_context)
	{
		plog::fatal("Failed to get second window's display context.\n");
		return false;
	}

	const int attrib_list[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0, // end of the list
	};

	uint32_t num_formats;

	wglChoosePixelFormatARB(display_context,
		attrib_list,
		nullptr,
		1,
		&pf,
		&num_formats);

	if (!SetPixelFormat(display_context, pf, &pfd))
	{
		plog::fatal("Failed to set second pixel format.\n");
		return false;
	}

	int context_flags = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	if (app::use_debug)
		context_flags |= WGL_CONTEXT_DEBUG_BIT_ARB;

	int context_attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, majorver,
		WGL_CONTEXT_MINOR_VERSION_ARB, minorver,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, context_flags,
		0, // end of the list
	};

	rendering_context = wglCreateContextAttribsARB(display_context, nullptr, context_attribs);
	if (!rendering_context)
	{
		plog::fatal("Failed to create second OpenGL context.\n");
		return false;
	}

	if (!wglMakeCurrent(display_context, rendering_context))
	{
		plog::fatal("Failed to activate second OpenGL context.\n");
		return false;
	}

	if (config.vsync)
		wglSwapIntervalEXT(1);
	else
		wglSwapIntervalEXT(0);

	plog::info("OpenGL %s.\n", glGetString(GL_VERSION));

	int context = 0;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &context);
	if (context & GL_CONTEXT_CORE_PROFILE_BIT)
		{ plog::infomore("Using Core profile.\n"); }
	else if (context & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
		{ plog::infomore("Using Compatability profile.\n"); }
	else
		{ plog::infomore("Using unknown profile?\n"); }

	context = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &context);
	if (context & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
		{ plog::infomore("Context is forward compatible.\n"); }
	else
		{ plog::infomore("Context is NOT forward compatible.\n"); }

	if (context & GL_CONTEXT_FLAG_DEBUG_BIT)
		{ plog::infomore("Debug context enabled.\n"); }
	else
		{ plog::infomore("Debug context disabled.\n"); }

	return true;
}

void ShutdownGraphics()
{
	wglMakeCurrent(display_context, nullptr);
	wglDeleteContext(rendering_context);
	rendering_context = nullptr;
}
#endif // RENDERER_OPENGL

} // namespace <anon>

bool Open()
{
	if (isOpen())
	{
		plog::fatal("Attempted to open a window which is already open.\n");
		return false;
	}

	// TODO: load window settings from config.xml

	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"Witchcraft";

	if (!RegisterClass(&wc))
	{
		plog::fatal("Failed to register window class: Error code %i.\n", GetLastError());
		return false;
	}

	window_handle = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		L"Witchcraft",
		utf8_to_utf16(app::window_title).c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		config.width, config.height,
		nullptr, nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	if (!window_handle)
	{
		plog::fatal("Failed to open a window: Error code %i.\n", GetLastError());
		return false;
	}

	if (!InitGraphics())
		{ return false; }

	if (config.maximized && !config.fullscreen)
		ShowWindow(window_handle, SW_SHOWMAXIMIZED);
	else
		ShowWindow(window_handle, SW_SHOW);

	if (config.fullscreen)
		setFullscreen(true);

	UpdateWindow(window_handle);
//	setMouseShown(false);

	// Register raw mouse input
	RAWINPUTDEVICE rid[1];
	rid[0].usUsagePage = 0x01;	// HID_USAGE_PAGE_GENERIC
	rid[0].usUsage = 0x02;		// HID_USAGE_GENERIC_MOUSE
	rid[0].dwFlags = RIDEV_INPUTSINK;
	rid[0].hwndTarget = window_handle;
	if (RegisterRawInputDevices(rid, 1, sizeof(rid[0])) == false)
	{
		plog::error("Failed to register raw mouse input:\n");
		plog::errmore("%s\n", GetLastError());
	}

	input::Init();

	running = true;
	active = true;
	return true;
}

void Close() {
	running = false;
	active = false;
	ShutdownGraphics();
	DestroyWindow(window_handle);
	window_handle = nullptr;

	// TODO: Save any config options that might have changed
}

bool isOpen()
	{ return (window_handle != nullptr); }

bool isActive()
	{ return active; }

void getWindowSize(int& w, int& h)
{
	RECT window_rect;
	GetWindowRect(window_handle, &window_rect);
	w = window_rect.right - window_rect.left;
	h = window_rect.bottom - window_rect.top;
}

void getDrawableSize(int& w, int& h)
{
	RECT window_rect;
	GetClientRect(window_handle, &window_rect);
	w = window_rect.right - window_rect.left;
	h = window_rect.bottom - window_rect.top;
}

bool isMouseShown()
	{ return mouse_shown; }

void setMouseShown(bool show)
{
	if ((show == true) && (mouse_shown == false))
	{
		mouse_shown = true;
		SetCursorPos(last_shown_mouse_x, last_shown_mouse_y);
		ShowCursor(true);
	}
	else if ((show == false) && (mouse_shown == true))
	{
		mouse_shown = false;
		POINT pos;
		GetCursorPos(&pos);
		last_shown_mouse_x = pos.x;
		last_shown_mouse_y = pos.y;
		ShowCursor(false);
	}
}

bool isFullscreen()
	{ return config.fullscreen; }

void setFullscreen(bool fullscreen)
{
	// TODO: just, all of this...
}

void setTitle(const string& title)
{
	if (!window_handle) return;
	SetWindowText(window_handle, utf8_to_utf16(title).c_str());
}


// Everything beyond this point is all about the Loop() function and input handling.


static unordered_map<uint32_t, InputButtonEnum> keyboard_mapping = 
{
	{ 'A', KEY_A },
	{ 'B', KEY_B },
	{ 'C', KEY_C },
	{ 'D', KEY_D },
	{ 'E', KEY_E },
	{ 'F', KEY_F },
	{ 'G', KEY_G },
	{ 'H', KEY_H },
	{ 'I', KEY_I },
	{ 'J', KEY_J },
	{ 'K', KEY_K },
	{ 'L', KEY_L },
	{ 'M', KEY_M },
	{ 'N', KEY_N },
	{ 'O', KEY_O },
	{ 'P', KEY_P },
	{ 'Q', KEY_Q },
	{ 'R', KEY_R },
	{ 'S', KEY_S },
	{ 'T', KEY_T },
	{ 'U', KEY_U },
	{ 'V', KEY_V },
	{ 'W', KEY_W },
	{ 'X', KEY_X },
	{ 'Y', KEY_Y },
	{ 'Z', KEY_Z },

	{ '1', KEY_1 },
	{ '2', KEY_2 },
	{ '3', KEY_3 },
	{ '4', KEY_4 },
	{ '5', KEY_5 },
	{ '6', KEY_6 },
	{ '7', KEY_7 },
	{ '8', KEY_8 },
	{ '9', KEY_9 },
	{ '0', KEY_0 },

	{ VK_SPACE, KEY_SPACE },
	{ VK_RETURN, KEY_ENTER },
	{ VK_OEM_MINUS, KEY_MINUS },
	{ VK_OEM_PLUS, KEY_EQUALS },
	{ VK_OEM_4, KEY_LBRACKET },
	{ VK_OEM_6, KEY_RBRACKET },
	{ VK_OEM_5, KEY_BACKSLASH },
	{ VK_OEM_1, KEY_SEMICOLON },
	{ VK_OEM_7, KEY_APOSTROPHE },
	{ VK_OEM_3, KEY_GRAVE },
	{ VK_OEM_COMMA, KEY_COMMA },
	{ VK_OEM_PERIOD, KEY_PERIOD },
	{ VK_OEM_2, KEY_SLASH },
	{ VK_CAPITAL, KEY_CAPSLOCK },

	{ VK_UP, KEY_UP },
	{ VK_DOWN, KEY_DOWN },
	{ VK_LEFT, KEY_LEFT },
	{ VK_RIGHT, KEY_RIGHT },

	{ VK_SHIFT, KEY_SHIFT },
	{ VK_CONTROL, KEY_CONTROL },
	{ VK_MENU, KEY_ALT },
	{ VK_LSHIFT, KEY_LSHIFT },
	{ VK_LCONTROL, KEY_LCONTROL },
	{ VK_LMENU, KEY_LALT },
	{ VK_RSHIFT, KEY_RSHIFT },
	{ VK_RCONTROL, KEY_RCONTROL },
	{ VK_RMENU, KEY_RALT },

	{ VK_F1, KEY_F1 },
	{ VK_F2, KEY_F2 },
	{ VK_F3, KEY_F3 },
	{ VK_F4, KEY_F4 },
	{ VK_F5, KEY_F5 },
	{ VK_F6, KEY_F6 },
	{ VK_F7, KEY_F7 },
	{ VK_F8, KEY_F8 },
	{ VK_F9, KEY_F9 },
	{ VK_F10, KEY_F10 },
	{ VK_F11, KEY_F11 },
	{ VK_F12, KEY_F12 },
};

bool Loop()
{
	if (!window_handle) return false;

	prev_input_state = current_input_state;
	current_input_state.clear();

	MSG msg;
	while (PeekMessage(&msg, window_handle, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			running = false;
			return running;
		}

		TranslateMessage(&msg);
		SizingCheck(&msg);
		DispatchMessage(&msg);
	}

	// Get this frame's keyboard and mouse button state.
	uint8_t keyboard_state[256];
	GetKeyboardState(keyboard_state);
	for (auto& it : keyboard_mapping)
	{
		if (keyboard_state[it.first] & (1 << 7))
		{
			current_input_state.keyboard[it.second] = true;
		}
	}

	// Mouse buttons.
	if (keyboard_state[VK_LBUTTON])
		current_input_state.mouse.buttons |= MOUSE_BUTTON_LEFT;

	if (keyboard_state[VK_RBUTTON])
		current_input_state.mouse.buttons |= MOUSE_BUTTON_RIGHT;

	if (keyboard_state[VK_MBUTTON])
		current_input_state.mouse.buttons |= MOUSE_BUTTON_MIDDLE;

	// Mouse position.
	if (mouse_shown)
	{
		POINT cursor_pos;
		GetCursorPos(&cursor_pos);
		current_input_state.mouse.x = cursor_pos.x;
		current_input_state.mouse.y = cursor_pos.y;
		current_input_state.mouse.delta_x = current_input_state.mouse.x - prev_input_state.mouse.x;
		current_input_state.mouse.delta_y = current_input_state.mouse.y - prev_input_state.mouse.y;
	}
	else
	{
		current_input_state.mouse.x = last_shown_mouse_x;
		current_input_state.mouse.y = last_shown_mouse_y;
		// If mouse is not shown, mouse delta is calculated in WndProc using raw input data.
	}

	// Mouse Wheel is calculated in WndProc and accumulated over the frame.
	// Some sanity checking here, in case of integer overflow.
	if (current_input_state.mouse.wheel_x - prev_input_state.mouse.wheel_x > 30000 ||
		current_input_state.mouse.wheel_x - prev_input_state.mouse.wheel_x < -30000)
		 { prev_input_state.mouse.wheel_x = current_input_state.mouse.wheel_x; }
	if (current_input_state.mouse.wheel_y - prev_input_state.mouse.wheel_y > 30000 ||
		current_input_state.mouse.wheel_y - prev_input_state.mouse.wheel_y < -30000)
		 { prev_input_state.mouse.wheel_y = current_input_state.mouse.wheel_y; }
	
	// XInput Controller
	XINPUT_STATE xinput;
	DWORD result = XInputGetState(0, &xinput);

	if (result == ERROR_SUCCESS && !controller_connected)
	{
		plog::info("XInput controller connected.\n");
		controller_connected = true;
	}
	if (result != ERROR_SUCCESS && controller_connected)
	{
		plog::info("XInput controller disconnected.\n");
		controller_connected = false;
	}

	if (controller_connected)
	{
		// Get axes.
		current_input_state.gamepad.lthumb.x = (float)xinput.Gamepad.sThumbLX / (float)SHRT_MAX;
		current_input_state.gamepad.lthumb.y = (float)xinput.Gamepad.sThumbLY / (float)SHRT_MAX;

		current_input_state.gamepad.rthumb.x = (float)xinput.Gamepad.sThumbRX / (float)SHRT_MAX;
		current_input_state.gamepad.rthumb.y = (float)xinput.Gamepad.sThumbRY / (float)SHRT_MAX;

		current_input_state.gamepad.ltrigger = (float)xinput.Gamepad.bLeftTrigger / (float)UCHAR_MAX;
		current_input_state.gamepad.rtrigger = (float)xinput.Gamepad.bRightTrigger / (float)UCHAR_MAX;

		// Gamepad buttons.
		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_A)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_A;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_B;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_X)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_X;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_Y;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_START)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_START;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_BACK;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_LEFT_SHOULDER;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_RIGHT_SHOULDER;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_LEFT_THUMB;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_RIGHT_THUMB;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_DPAD_UP;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_DPAD_DOWN;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_DPAD_LEFT;

		if (xinput.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
			current_input_state.gamepad.buttons |= GAMEPAD_BUTTON_DPAD_RIGHT;

	}

	input::Process(prev_input_state, current_input_state);

	return running;
}

void Display()
	{ SwapBuffers(display_context); }

const InputState& getInputState()
	{ return current_input_state; }

} // namespace Window

#endif // PLATFORM_WIN32