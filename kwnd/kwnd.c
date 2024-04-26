#include <kwnd/kwnd.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef struct kwnd_global {
	s8 fail;
	kwnd_error_t backupError;
	kwnd_error_t* errors;
	u32 error_count;
	u32 error_capacity;
} kwnd_global_t;

static kwnd_global_t global = { 0 };

static inline void kwnd_error_push(kwnd_error_t error);
static inline void kwnd_event_push(kwnd_window_t* window, kwnd_event_t event);

static LRESULT CALLBACK kwnd_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#ifndef FUNC_NAME
	#ifdef WIN32
		#define FUNC_NAME __FUNCTION__
	#elif __STDC_VERSION__ >= 199901L
		#define FUNC_NAME __func__
	#elif defined(__PRETTY_FUNCTION__)
		#define FUNC_NAME __PRETTY_FUNCTION__
	#else
		#define FUNC_NAME "unknown"
	#endif
#endif

#define KWND_ERROR(c, m) kwnd_error_push((kwnd_error_t) { .code = c, .message = m, .from = FUNC_NAME })

kwnd_error_code_t kwnd_window_create(kwnd_window_t* out_window, const char* title, int width, int height) {
	if (global.fail) {
		return KWND_FATAL;
	}

	if (out_window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "out_window must not be null");
	}

	if (title == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "title must not be null");
	}

	if (width <= 0) {
		KWND_ERROR(KWND_ERROR_INVALID_ARGUMENT, "width must be greater than 0");
	}

	if (height <= 0) {
		KWND_ERROR(KWND_ERROR_INVALID_ARGUMENT, "height must be greater than 0");
	}

	WNDCLASSEXA wnd_class = {
		.cbSize = sizeof(WNDCLASSEXA),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = kwnd_window_proc,
		.hInstance = GetModuleHandleA(NULL),
		.hIcon = LoadIconA(NULL, IDI_APPLICATION),
		.hCursor = LoadCursorA(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1),
		.lpszClassName = "kwnd_window_class"
	};

	if (!RegisterClassExA(&wnd_class)) {
		KWND_ERROR(KWND_ERROR_WINDOW_CREATION_FAILURE, "failed to register window class");
		return KWND_ERROR_WINDOW_CREATION_FAILURE;
	}

	RECT rect = {
		.left = 0,
		.top = 0,
		.right = width,
		.bottom = height
	};
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

	HWND hwnd = CreateWindowExA(0,
		wnd_class.lpszClassName, title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL,
		wnd_class.hInstance, NULL
	);

	if (hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_WINDOW_CREATION_FAILURE, "failed to create window");
		return KWND_ERROR_WINDOW_CREATION_FAILURE;
	}

	SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR) out_window);

	memset(out_window, 0, sizeof(kwnd_window_t));
	out_window->platform.hwnd = hwnd;
	out_window->platform.hinstance = wnd_class.hInstance;
	out_window->platform.wnd_class = wnd_class;
	out_window->width = width;
	out_window->height = height;

	return KWND_SUCCESS;
}

void kwnd_window_destroy(kwnd_window_t* window) {
	if (window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "window must not be null");
		return;
	}

	if (window->platform.hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_WINDOW, "window.platform.hwnd must be valid");
		return;
	}

	DestroyWindow(window->platform.hwnd);
	UnregisterClassA(window->platform.wnd_class.lpszClassName, window->platform.hinstance);

	memset(window, 0, sizeof(kwnd_window_t));
}

void kwnd_update_window(kwnd_window_t* window) {
	if (global.fail) {
		return;
	}

	if (window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "window must not be null");
		return;
	}

	if (window->platform.hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_WINDOW, "window.platform.hwnd must be valid");
		return;
	}

	window->internal.event_count = 0;

	MSG msg;
	while (PeekMessageA(&msg, window->platform.hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	window->internal.total_event_count_since_last_resize += window->internal.event_count;
}

kwnd_error_code_t kwnd_poll_event(kwnd_window_t* window, kwnd_event_t* out_event) {
	if (global.fail) {
		return KWND_FATAL;
	}

	if (window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "window must not be null");
		return KWND_ERROR_INVALID_POINTER;
	}

	if (window->platform.hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_WINDOW, "window.platform.hwnd must be valid");
		return KWND_ERROR_INVALID_WINDOW;
	}

	if (out_event == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "out_event must not be null");
		return KWND_ERROR_INVALID_POINTER;
	}

	if (window->internal.event_count == 0) {
		return KWND_SOFT_ERROR_NOTHING_IN_QUEUE;
	}

	++window->internal.iterations_since_last_event_resize;
	*out_event = window->internal.events[--window->internal.event_count];
	if (window->internal.event_count < window->internal.event_capacity / 2 && window->internal.event_count != 0) {
		window->internal.event_capacity /= 2;
		window->internal.events = realloc(window->internal.events, sizeof(kwnd_event_t) * window->internal.event_capacity);
		window->internal.iterations_since_last_event_resize = 0;
		window->internal.total_event_count_since_last_resize = 0;
	}

	return KWND_SUCCESS;
}

void kwnd_show_window(kwnd_window_t* window) {
	if (global.fail) {
		return KWND_FATAL;
	}

	if (window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "window must not be null");
		return;
	}

	if (window->platform.hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_WINDOW, "window.platform.hwnd must be valid");
		return;
	}

	if (window->visible) {
		return;
	}

	ShowWindow(window->platform.hwnd, SW_SHOW);
}

void kwnd_hide_window(kwnd_window_t* window) {
	if (global.fail) {
		return KWND_FATAL;
	}

	if (window == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_POINTER, "window must not be null");
		return;
	}

	if (window->platform.hwnd == NULL) {
		KWND_ERROR(KWND_ERROR_INVALID_WINDOW, "window.platform.hwnd must be valid");
		return;
	}

	if (!window->visible) {
		return;
	}

	ShowWindow(window->platform.hwnd, SW_HIDE);
}

int kwnd_error_pop(kwnd_error_t* out_error) {
	if (global.fail) {
		*out_error = global.backupError;
		return KWND_SUCCESS;
	}

	if (out_error == NULL) {
		return KWND_ERROR_INVALID_POINTER;
	}

	if (global.error_count == 0) {
		return KWND_SOFT_ERROR_NOTHING_IN_QUEUE;
	}

	*out_error = global.errors[--global.error_count];
	if (global.error_count < global.error_capacity / 2 && global.error_count != 0) {
		global.error_capacity /= 2;
		global.errors = realloc(global.errors, sizeof(kwnd_error_t) * global.error_capacity);
	}

	if (global.errors == NULL) {
		KWND_ERROR(KWND_FATAL_OUT_OF_MEMORY, "out of memory");
		return KWND_FATAL_OUT_OF_MEMORY;
	}

	return KWND_SUCCESS;
}

static inline void kwnd_error_push(kwnd_error_t error) {
	if (global.fail) {
		return;
	}

	if (error.code >= KWND_FATAL) {
		global.fail = 1;
		global.backupError = error;
		return;
	}

	if (global.error_count == global.error_capacity) {
		global.error_capacity = global.error_capacity == 0 ? 1 : (global.error_capacity * 3) / 2;

		if (global.errors == NULL) {
			global.errors = malloc(sizeof(kwnd_error_t) * global.error_capacity);
		} else {
			global.errors = realloc(global.errors, sizeof(kwnd_error_t) * global.error_capacity);
		}

		if (global.errors == NULL) {
			KWND_ERROR(KWND_FATAL_OUT_OF_MEMORY, "out of memory");
			return;
		}
	}

	global.errors[global.error_count++] = error;
}

static inline void kwnd_event_push(kwnd_window_t* window, kwnd_event_t event) {
	if (global.fail) {
		return;
	}

	if (window->internal.event_count == window->internal.event_capacity) {
		window->internal.event_capacity = window->internal.event_capacity == 0 ? 1 : (window->internal.event_capacity * 3) / 2;

		++window->internal.iterations_since_last_event_resize;
		if (window->internal.events == NULL) {
			window->internal.events = malloc(sizeof(kwnd_event_t) * window->internal.event_capacity);
		} else {
			window->internal.events = realloc(window->internal.events, sizeof(kwnd_event_t) * window->internal.event_capacity);
			window->internal.iterations_since_last_event_resize = 0;
			window->internal.total_event_count_since_last_resize = 0;
		}

		if (window->internal.events == NULL) {
			KWND_ERROR(KWND_FATAL_OUT_OF_MEMORY, "out of memory");
			return;
		}
	}

	window->internal.events[window->internal.event_count++] = event;
}

static inline kwnd_keycode_t kwnd_win32_key_to_keycode(UINT key) {
    switch (key) {
		case VK_ESCAPE: return KWND_KEYCODE_ESCAPE;
		case VK_SPACE: return KWND_KEYCODE_SPACE;
		case 'A': return KWND_KEYCODE_A;
		case 'B': return KWND_KEYCODE_B;
		case 'C': return KWND_KEYCODE_C;
		case 'D': return KWND_KEYCODE_D;
		case 'E': return KWND_KEYCODE_E;
		case 'F': return KWND_KEYCODE_F;
		case 'G': return KWND_KEYCODE_G;
		case 'H': return KWND_KEYCODE_H;
		case 'I': return KWND_KEYCODE_I;
		case 'J': return KWND_KEYCODE_J;
		case 'K': return KWND_KEYCODE_K;
		case 'L': return KWND_KEYCODE_L;
		case 'M': return KWND_KEYCODE_M;
		case 'N': return KWND_KEYCODE_N;
		case 'O': return KWND_KEYCODE_O;
		case 'P': return KWND_KEYCODE_P;
		case 'Q': return KWND_KEYCODE_Q;
		case 'R': return KWND_KEYCODE_R;
		case 'S': return KWND_KEYCODE_S;
		case 'T': return KWND_KEYCODE_T;
		case 'U': return KWND_KEYCODE_U;
		case 'V': return KWND_KEYCODE_V;
		case 'W': return KWND_KEYCODE_W;
		case 'X': return KWND_KEYCODE_X;
		case 'Y': return KWND_KEYCODE_Y;
		case 'Z': return KWND_KEYCODE_Z;
		case '0': return KWND_KEYCODE_0;
		case '1': return KWND_KEYCODE_1;
		case '2': return KWND_KEYCODE_2;
		case '3': return KWND_KEYCODE_3;
		case '4': return KWND_KEYCODE_4;
		case '5': return KWND_KEYCODE_5;
		case '6': return KWND_KEYCODE_6;
		case '7': return KWND_KEYCODE_7;
		case '8': return KWND_KEYCODE_8;
		case '9': return KWND_KEYCODE_9;
		case VK_NUMPAD0: return KWND_KEYCODE_NUMPAD_0;
		case VK_NUMPAD1: return KWND_KEYCODE_NUMPAD_1;
        case VK_NUMPAD2: return KWND_KEYCODE_NUMPAD_2;
		case VK_NUMPAD3: return KWND_KEYCODE_NUMPAD_3;
		case VK_NUMPAD4: return KWND_KEYCODE_NUMPAD_4;
		case VK_NUMPAD5: return KWND_KEYCODE_NUMPAD_5;
		case VK_NUMPAD6: return KWND_KEYCODE_NUMPAD_6;
		case VK_NUMPAD7: return KWND_KEYCODE_NUMPAD_7;
		case VK_NUMPAD8: return KWND_KEYCODE_NUMPAD_8;
		case VK_NUMPAD9: return KWND_KEYCODE_NUMPAD_9;
		case VK_F1: return KWND_KEYCODE_F1;
		case VK_F2: return KWND_KEYCODE_F2;
		case VK_F3: return KWND_KEYCODE_F3;
		case VK_F4: return KWND_KEYCODE_F4;
		case VK_F5: return KWND_KEYCODE_F5;
		case VK_F6: return KWND_KEYCODE_F6;
		case VK_F7: return KWND_KEYCODE_F7;
		case VK_F8: return KWND_KEYCODE_F8;
		case VK_F9: return KWND_KEYCODE_F9;
		case VK_F10: return KWND_KEYCODE_F10;
		case VK_F11: return KWND_KEYCODE_F11;
		case VK_F12: return KWND_KEYCODE_F12;
		case VK_LSHIFT: return KWND_KEYCODE_LEFT_SHIFT;
		case VK_RSHIFT: return KWND_KEYCODE_RIGHT_SHIFT;
		case VK_LCONTROL: return KWND_KEYCODE_LEFT_CONTROL;
		case VK_RCONTROL: return KWND_KEYCODE_RIGHT_CONTROL;
		case VK_LMENU: return KWND_KEYCODE_LEFT_ALT;
		case VK_RMENU: return KWND_KEYCODE_RIGHT_ALT;
		case VK_LWIN: return KWND_KEYCODE_LEFT_SUPER;
		case VK_RWIN: return KWND_KEYCODE_RIGHT_SUPER;
		case VK_APPS: return KWND_KEYCODE_OPTION;
		default: return -1;
	}
}

static LRESULT CALLBACK kwnd_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	kwnd_window_t* window = (kwnd_window_t*) GetWindowLongPtrA(hwnd, GWLP_USERDATA);
	if (window == NULL) {
		return DefWindowProcA(hwnd, msg, wparam, lparam);
	}

	switch (msg) {
		case WM_DESTROY:
			PostQuitMessage(0);
			window->closed = 1;
			break;
		case WM_CLOSE:
			kwnd_event_push(window, (kwnd_event_t) {
				.type = KWND_EVENT_CLOSE,
			});
			break;
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);
			window->width = rect.right - rect.left;
			window->height = rect.bottom - rect.top;

			if (window->width == 0 && window->height == 0) {
				window->minimized = 1;
				kwnd_event_push(window, (kwnd_event_t) {
					.type = KWND_EVENT_MINIMIZE,
				});
				break;
			} else {
				if (window->minimized) {
					kwnd_event_push(window, (kwnd_event_t) {
						.type = KWND_EVENT_MINIMIZE,
					});
					break;
				}
				window->minimized = 0;
			}

			kwnd_event_push(window, (kwnd_event_t) {
				.type = KWND_EVENT_RESIZE,
			});
			break;
		}
		case WM_KEYDOWN:
		case WM_KEYUP: {
			kwnd_event_push(window, (kwnd_event_t) {
				.type = KWND_EVENT_KEY,
				.data.key = {
					.keycode = kwnd_win32_key_to_keycode(wparam),
					.pressed = msg == WM_KEYDOWN
				}
			});
			break;
		}
		default:
			return DefWindowProcA(hwnd, msg, wparam, lparam);
	}

	return 0;
}