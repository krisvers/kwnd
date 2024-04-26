#ifndef KRISVERS_KWND_WIN32_H
#define KRISVERS_KWND_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct kwnd_window_platform {
	HWND hwnd;
	HINSTANCE hinstance;
	WNDCLASSEXA wnd_class;
} kwnd_window_platform_t;

#endif // KRISVERS_KWND_WIN32_H