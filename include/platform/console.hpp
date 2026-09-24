#pragma once

enum console_color : signed char {
	white_color = 0x00,
	red_color = 0x01,
	green_color = 0x02,
	blue_color = 0x03
};

size_t platform_console_write(const char* buffer, size_t count) noexcept;
void platform_console_color(console_color color);

struct console_basic_state {
	console_color color;
};

[[nodiscard]] console_basic_state platform_console_basic_state() noexcept;