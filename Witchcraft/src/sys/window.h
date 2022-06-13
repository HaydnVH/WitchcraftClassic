#ifndef HVH_WC_SYS_WINDOW_H
#define HVH_WC_SYS_WINDOW_H

#include <string>
#include "input.h"

namespace window
{
	/* Opens a window, unless a window is already open. */
	/* Must initialize after: Config. */
	/* Returns false if a window failed to open. */
	bool Open();
	/* Closes the window, unless there is no window to close. */
	void Close();

	/* Returns whether the window is currently open. */
	bool isOpen();
	/* Returns whether or not the window is currently active. */
	bool isActive();

	/* Performs message handling and processes user input. Returns false when the program should terminate. */
	bool Loop();
	/* Performs a buffer swap to allow the currently drawn frame to be presented to the user. */
	void Display();

	/* Sets the title of the window (utf-8). */
	void setTitle(const std::string& title);
	/* Sets the input references 'w' and 'h' to be the width and height of the window, respectively. */
	void getWindowSize(int& w, int& h);
	/* Sets the input references 'w' and 'h' to be the dimensions of the window's drawable area. */
	void getDrawableSize(int& w, int& h);

	/* Returns whether or not the mouse cursor is currently shown or hidden. */
	bool isMouseShown();
	/* Hides or reveals the mouse cursor. */
	void setMouseShown(bool show);

	/* Returns whether or not the window is currently in Fullscreen mode. */
	bool isFullscreen();
	/* Sets or disables Fullscreen mode.*/
	void setFullscreen(bool fullscreen);

	/* Gets the current input state as of the most recent update. */
	const InputState& getInputState();
}

#endif // HVH_WC_SYS_WINDOW_H