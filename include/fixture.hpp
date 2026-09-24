#pragma once

namespace unitt
{
    struct no_fixture_t {};
    constexpr inline no_fixture_t no_fixture{};

    template <typename Symbol>
    struct fixture_creator {
        constexpr static no_fixture_t create() noexcept {
            return no_fixture;
        }
    };
}