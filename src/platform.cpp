#include "platform.hpp"

#if defined(_WIN32)

	#include <Windows.h>

	size_t platform_console_write(const char* buffer, size_t count) noexcept {
		const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

		DWORD written;
		WriteConsole(console, buffer, count, &written, 0);

		return written;
	}

	void platform_console_color(console_color color) {
		const HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		if (color == white_color) {
			SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		if (color == red_color) {
			SetConsoleTextAttribute(console, FOREGROUND_RED);
		}
		else if (color == green_color) {
			SetConsoleTextAttribute(console, FOREGROUND_GREEN);
		}
		else if (color == blue_color) {
			SetConsoleTextAttribute(console, FOREGROUND_BLUE);
		}
	}

#endif