#pragma once

#include <cstdint>

namespace unitt
{
    enum class testing_event {
        group_start,
        test_start,
        assertion,
        message,
        test_end,
        group_end
    };

    template <testing_event, typename Placeholder = void>
    struct event_info {};

    template <typename Group>
    struct event_info<testing_event::group_start, Group> {
        constexpr static inline std::string_view name = Group::name;
    };

    template <>
    struct event_info<testing_event::assertion> {
        /// @brief Type of the assertion.
        assertion_result result{};
        assertion_type type{};
    };

    template <>
    struct event_info<testing_event::message> {
        /// @brief Message content.
        std::string_view message{};
    };

    template <>
    struct event_info<testing_event::test_start> {
        /// @brief Name of the test, it references a string with static storage duration.
        std::string_view name{};
    };

    template <>
    struct event_info<testing_event::test_end> {
        /// @brief Either failure (0), or success (1).
        bool result{};
    };

    template <>
    struct event_info<testing_event::group_start> {
        /// @brief Either failure (0), or success (1).
        /// Failure means that at least one test contained within the group was a failure.
        /// Success is the opposite of failure (no tests in the group were a failure).
        bool result{};
    };

    using group_start_info = event_info<testing_event::group_start>;
    using test_start_info = event_info<testing_event::test_start>;
    using assertion_info = event_info<testing_event::assertion>;
    using message_info = event_info<testing_event::message>;
    using test_end_info = event_info<testing_event::test_end>;
    using group_end_info = event_info<testing_event::group_end>;

    // TODO: Move to a different file.
    class default_formatter {
    public:
        template <testing_event Event, typename Bonus>
        void comment(unitt::execution execi, event_info<Event, Bonus> info) noexcept;

    private:
        uint16_t group_label_id{};
        uint16_t test_label_id{};
    };
}