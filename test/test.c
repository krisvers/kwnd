#include <kwnd/kwnd.h>
#include <stdio.h>

int main() {
	kwnd_window_t window;
	if (kwnd_window_create(&window, "Hello, World!", 800, 600) != KWND_SUCCESS) {
		kwnd_error_t error;
		if (kwnd_error_pop(&error) == KWND_SUCCESS) {
			printf("Error (%s): %s\n", error.from, error.message);
		}
		return 1;
	}

	kwnd_show_window(&window);

	while (!window.closed) {
		kwnd_event_t event;
		kwnd_update_window(&window);
		while (kwnd_poll_event(&window, &event) == KWND_SUCCESS) {
			if (event.type == KWND_EVENT_KEY) {
				if (event.data.key.keycode == KWND_KEYCODE_ESCAPE) {
					window.closed = 1;
				}
			}
		}
	}

	kwnd_window_destroy(&window);
	return 0;
}