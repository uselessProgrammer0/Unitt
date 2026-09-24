#pragma once

#include "platform/console.hpp"

namespace unitt
{
    enum class assertion_type : signed char {
        eq,  // ==
        neq, // !=
        lt,  // <
        gt,  // >
        le,  // <=
        ge,  // >=
        throws
    };

    enum class assertion_result : signed char {
        success = green_color,
        failure = red_color,
        neutral = blue_color // That's not even really an assertion, we just make it an assertion to universalize behavior.
    };
}