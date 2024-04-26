#ifndef KRISVERS_KWND_H
#define KRISVERS_KWND_H

#ifdef WIN32
#include <kwnd/kwnd_win32.h>
#else
#error "Unsupported platform"
#endif // WIN32

#include <stdio.h>

typedef enum kwnd_error_code {
	/* soft errors */
	KWND_SOFT_ERROR_NOTHING_IN_QUEUE = -2,
	KWND_SOFT_ERROR = -1,

	/* success */
	KWND_ERROR_NONE = 0,
	KWND_SUCCESS = KWND_ERROR_NONE,

	/* errors */
	KWND_ERROR_INVALID_POINTER = 1,
	KWND_ERROR_INVALID_ARGUMENT = 2,
	KWND_ERROR_INVALID_WINDOW = 3,
	KWND_ERROR_WINDOW_CREATION_FAILURE = 4,

	/* fatal */
	KWND_FATAL = 111, /* fatal error happened prior, cannot continue execution */
	KWND_FATAL_OUT_OF_MEMORY = 127,
} kwnd_error_code_t;

typedef enum kwnd_event_type {
	KWND_EVENT_NONE = 0,
	KWND_EVENT_KEY = 1,
	KWND_EVENT_RESIZE = 2,
	KWND_EVENT_CLOSE = 3,
	KWND_EVENT_MINIMIZE = 4,
} kwnd_event_type_t;

typedef enum kwnd_keycode {
	/* control keys */
	KWND_KEYCODE_LEFT_SHIFT = 16,
	KWND_KEYCODE_LEFT_CONTROL = 17,
	KWND_KEYCODE_LEFT_ALT = 18,
	KWND_KEYCODE_RIGHT_SHIFT = 19,
	KWND_KEYCODE_RIGHT_CONTROL = 20,
	KWND_KEYCODE_RIGHT_ALT = 21,
	KWND_KEYCODE_LEFT_SUPER = 22,
	KWND_KEYCODE_RIGHT_SUPER = 23,
	KWND_KEYCODE_OPTION = 24,

	/* character keys */
	KWND_KEYCODE_A = 65,
	KWND_KEYCODE_B = 66,
	KWND_KEYCODE_C = 67,
	KWND_KEYCODE_D = 68,
	KWND_KEYCODE_E = 69,
	KWND_KEYCODE_F = 70,
	KWND_KEYCODE_G = 71,
	KWND_KEYCODE_H = 72,
	KWND_KEYCODE_I = 73,
	KWND_KEYCODE_J = 74,
	KWND_KEYCODE_K = 75,
	KWND_KEYCODE_L = 76,
	KWND_KEYCODE_M = 77,
	KWND_KEYCODE_N = 78,
	KWND_KEYCODE_O = 79,
	KWND_KEYCODE_P = 80,
	KWND_KEYCODE_Q = 81,
	KWND_KEYCODE_R = 82,
	KWND_KEYCODE_S = 83,
	KWND_KEYCODE_T = 84,
	KWND_KEYCODE_U = 85,
	KWND_KEYCODE_V = 86,
	KWND_KEYCODE_W = 87,
	KWND_KEYCODE_X = 88,
	KWND_KEYCODE_Y = 89,
	KWND_KEYCODE_Z = 90,
	KWND_KEYCODE_0 = 48,
	KWND_KEYCODE_1 = 49,
	KWND_KEYCODE_2 = 50,
	KWND_KEYCODE_3 = 51,
	KWND_KEYCODE_4 = 52,
	KWND_KEYCODE_5 = 53,
	KWND_KEYCODE_6 = 54,
	KWND_KEYCODE_7 = 55,
	KWND_KEYCODE_8 = 56,
	KWND_KEYCODE_9 = 57,
	KWND_KEYCODE_SPACE = 32,
	KWND_KEYCODE_ENTER = 13,
	KWND_KEYCODE_BACKSPACE = 8,
	KWND_KEYCODE_TAB = 9,

	/* misc */
	KWND_KEYCODE_ESCAPE = 27,
	KWND_KEYCODE_LEFT = 37,
	KWND_KEYCODE_UP = 38,
	KWND_KEYCODE_RIGHT = 39,
	KWND_KEYCODE_DOWN = 40,
	KWND_KEYCODE_HOME = 36,
	KWND_KEYCODE_END = 35,
	KWND_KEYCODE_PAGE_UP = 33,
	KWND_KEYCODE_PAGE_DOWN = 34,
	KWND_KEYCODE_F1 = 112,
	KWND_KEYCODE_F2 = 113,
	KWND_KEYCODE_F3 = 114,
	KWND_KEYCODE_F4 = 115,
	KWND_KEYCODE_F5 = 116,
	KWND_KEYCODE_F6 = 117,
	KWND_KEYCODE_F7 = 118,
	KWND_KEYCODE_F8 = 119,
	KWND_KEYCODE_F9 = 120,
	KWND_KEYCODE_F10 = 121,
	KWND_KEYCODE_F11 = 122,
	KWND_KEYCODE_F12 = 123,
	KWND_KEYCODE_CAPS_LOCK = 20,
	KWND_KEYCODE_NUM_LOCK = 144,
	KWND_KEYCODE_SCROLL_LOCK = 145,
	KWND_KEYCODE_PRINT_SCREEN = 44,
	KWND_KEYCODE_PAUSE = 19,
	KWND_KEYCODE_MENU = 93,
	KWND_KEYCODE_SUPER = 91,
	KWND_KEYCODE_INSERT = 45,
	KWND_KEYCODE_DELETE = 46,
	KWND_KEYCODE_SEMICOLON = 59,
	KWND_KEYCODE_EQUALS = 61,
	KWND_KEYCODE_COMMA = 44,
	KWND_KEYCODE_MINUS = 45,
	KWND_KEYCODE_PERIOD = 46,
	KWND_KEYCODE_SLASH = 47,
	KWND_KEYCODE_BACKTICK = 96,
	KWND_KEYCODE_LEFT_BRACKET = 91,
	KWND_KEYCODE_BACKSLASH = 92,
	KWND_KEYCODE_RIGHT_BRACKET = 93,
	KWND_KEYCODE_QUOTE = 39,
	KWND_KEYCODE_MULTIPLY = 106,
	KWND_KEYCODE_ADD = 107,
	KWND_KEYCODE_SUBTRACT = 109,
	KWND_KEYCODE_DECIMAL = 110,
	KWND_KEYCODE_DIVIDE = 111,
	KWND_KEYCODE_NUMPAD_0 = 96,
	KWND_KEYCODE_NUMPAD_1 = 97,
	KWND_KEYCODE_NUMPAD_2 = 98,
	KWND_KEYCODE_NUMPAD_3 = 99,
	KWND_KEYCODE_NUMPAD_4 = 100,
	KWND_KEYCODE_NUMPAD_5 = 101,
	KWND_KEYCODE_NUMPAD_6 = 102,
	KWND_KEYCODE_NUMPAD_7 = 103,
	KWND_KEYCODE_NUMPAD_8 = 104,
	KWND_KEYCODE_NUMPAD_9 = 105,
	KWND_KEYCODE_NUMPAD_EQUALS = 13,
	KWND_KEYCODE_NUMPAD_ENTER = 13,
	KWND_KEYCODE_NUMPAD_ADD = 107,
	KWND_KEYCODE_NUMPAD_SUBTRACT = 109,
} kwnd_keycode_t;

typedef struct kwnd_error {
	kwnd_error_code_t code;
	const char* message;
	const char* from;
} kwnd_error_t;

typedef struct kwnd_event {
	kwnd_event_type_t type;
	union {
		struct {
			float x;
			float y;
		} mouse_pos;
		struct {
			kwnd_keycode_t keycode;
			int pressed;
		} key;
	} data;
} kwnd_event_t;

typedef struct kwnd_window_internal {
	kwnd_event_t* events;
	unsigned int event_count;
	unsigned int event_capacity;
	unsigned int iterations_since_last_event_resize;
	unsigned int total_event_count_since_last_resize;
} kwnd_window_internal_t;

typedef struct kwnd_window {
	unsigned int width;
	unsigned int height;
	int closed;
	int visible;
	int minimized;

	kwnd_window_platform_t platform;
	kwnd_window_internal_t internal;
} kwnd_window_t;

kwnd_error_code_t kwnd_window_create(kwnd_window_t* out_window, const char* title, int width, int height);
void kwnd_window_destroy(kwnd_window_t* window);
void kwnd_update_window(kwnd_window_t* window);
kwnd_error_code_t kwnd_poll_event(kwnd_window_t* window, kwnd_event_t* out_event);
void kwnd_show_window(kwnd_window_t* window);
void kwnd_hide_window(kwnd_window_t* window);

kwnd_error_code_t kwnd_error_pop(kwnd_error_t* out_error);
void kwnd_error_fprint(FILE* fp, kwnd_error_t* error);

#endif // KRISVERS_KWND_H