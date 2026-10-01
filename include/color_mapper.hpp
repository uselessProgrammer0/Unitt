#pragma once

#include "platform/console.hpp"
#include "spdarr.hpp"

#include <cstdint>

namespace unitt
{
    class color_mapper {
    public:
        constexpr color_mapper(console_color initial) noexcept {
            colors.push_back(initial);
            areas.push_back(0);
        }

        struct entry {
            console_color color{};
            uint32_t area{};
        };

    public:
        /// @brief Register new color, when active color is same as requested color, nothing happens.
        ///   Otherwise, new color entry is created.
        ///   In general, after calling this function, color of next message is guaranteed to be the requested one.
        ///   You can call this after calling reserve, it will extend the area in case color is same as reserved area's initial color, 
        ///     otherwise it will be equivalent to calling end_reserve.
        constexpr void request(console_color color) noexcept {
            if (colors.back() != color) {
                colors.push_back(color);
                areas.push_back(0);
            }
        }

        /// @brief Create new color entry, you should only use it if you will change the color of that entry later.
        ///   If you want to end the area which will be affected by alter_color_area with returned ID passed, call end_reserve_color.
        /// @returns ID value which you can pass to alter_color_area to change the color value of area created by this function.
        ///   The returned ID is always different than 0, so you can use 0 as null value.
        [[nodiscard]] constexpr uint16_t reserve(console_color color) noexcept {
            // Create new entry, even if provided color is same as active (last entry) color,
            //   this will allow to change color of that entry later without affecting area of the previous entry.
            // In case the color won't be changed, we will lose some speed
            //   (two entries for two same colors next to each other, normally that would be just one entry), but that's the job of user to ensure that.
            colors.push_back(color);
            areas.push_back(0);

            return colors.size() - 1; // == areas.size() - 1
        }

        /// @brief Marks the end of the area which will be affected by calling alter_area with id obtained from preceding reserve call.
        ///   You shouldn't call this function without previously calling reserve.
        constexpr void end_reserve(console_color color) noexcept {
            colors.push_back(color);
            areas.push_back(0);
        }

        /// @brief Get color of last added entry.
        [[nodiscard]] constexpr console_color get_active() const noexcept {
            return colors.back();
        }

        /// @brief Change color of previously reserved color area via call to reserve function.
        constexpr void alter_area(size_t id, console_color color) noexcept {
        #if UNITT_SLOW
            // When there are two same colors next to each other, then ideally, we want the new color to be different than the one next to it.
            // This not an error though, it's okay logic-wise for this to happen, but we will make more iterations accessing less characters each iteration.
            // TODO: We can do a finalizing sweep which would merge same color entries laying next to each other?
            if (colors.size() > 1 && id) {
                assert((color != colors[id - 1]) // TODO: We don't want to assert, we want to warn.
                    && "alter_color_area: promise not fulfilled; set color to a different value than it initially was set via reserve_color.");
            }
        #endif
            colors[id] = color;
        }

        /// @brief Increase number of characters in the active entry area by chars amount.
        constexpr void extend_active(uint32_t chars) noexcept {
            areas.back() += chars;
        }
       
        [[nodiscard]] constexpr size_t size() const noexcept {
            return areas.size(); // == colors.size()
        }

        [[nodiscard]] constexpr entry get_entry(size_t index) const noexcept {
            return { colors[index], areas[index] };
        }

    private:
        // Every color from colors is active since the index equal to the value of accumulate(*all previous areas values*) for areas[index] characters.
        // So given example areas = { 7, 45, 36, 140, 9 } and example colors = { red, blue, red, green, red }, green color is active since 7 + 45 + 36 index, 
        //   for 140 characters in the output.
        // Also, there can't be two same colors next to each other, because area of "previous" (same as "current") color is just extended in such case, 
        //   and size of colors is always equal to size of areas.
        // This exists because we want to perform system calls as rarely as possible, while still being able to display various colors.
        spdarr<uint32_t, 256> areas{};
        spdarr<console_color, 256> colors{}; // TODO: Create something like ioparr, range capable of storing sub-byte integer values per "index", since we will use just 3 colors (so 3 bits) for each index.
    };
}