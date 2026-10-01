#pragma once

#include "color_mapper.hpp"
#include "assert.hpp"

#include <vector>
#include <string_view>
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

    struct string_encoder_result {
        std::vector<char> chars{};
        color_mapper colors{ blue_color };
    };

    /// @brief Listen to testing events and convert them to human readable text.
    /// This is the default encoder used by unitt, it provides data and interface to store the text and colors.
    /// @tparam Formatter Formatter used to comment testing events, it is the thing that actually builds the output.
    /// Every event is delegated to formatter which can then use messenger provided by this encoder to store messages inside it.
    template <typename Formatter = default_string_encoder_formatter>
    class string_encoder;

    struct string_encoder_data {
        color_mapper colors{ blue_color };
        std::vector<char> chars{};
    };

    class string_encoder_proxy {
    public:
        string_encoder_proxy(string_encoder_data& encoder_data)
            : encoder_data_ref { encoder_data }, colors { encoder_data.colors } {}

        template <typename... Args>
        void fmessage(std::string_view format, const Args&... fmtargs) {
            encoder_ref.fmessage(format, std::forward<Args>(fmtargs)...);
        }

    private:
        string_encoder_data& encoder_data_ref;

    public:
        color_mapper& colors;
    };

    template <typename Formatter>
    class string_encoder {
    public:
        void initialize() {
            // For the start we allocate four megabytes of memory for characters, vector will grow automatically if that's not enough.
            constexpr size_t characters = 4 * 1024 * 1024;
            data.chars.reserve(characters);
        }
        
        // BonusData is typename that is provided by unitt in some cases when type is the main source of information, 
        //   or is at least required/helpful in retrieving such data, for example group typename is required in some function calls, 
        //   so it will be provided in case of group_start/group_summary events.
        template <testing_event Event, typename BonusData>
        void encode(const execution_info& execution, const event_info<Event, BonusData>& event) noexcept {
            string_encoder_messenger<Formatter> messenger{ *this };
        }

        template <typename... Args>
        void fmessage(std::string_view format, const Args&... fmtargs) {
            const size_t offset = chars.size();
            chars.resize(chars.size() + indentation.value, indentation.symbol);
            if constexpr (sizeof...(Args) == 0) {
                std::copy_n(format.begin(), format.size(), std::back_inserter(chars));
            }
            else {
                std::vformat_to(std::back_inserter(chars), format, std::make_format_args(fmtargs...));
            }

            colors.extend_active(chars.size() - offset);
        }

        string_encoder_result build() noexcept {
            // Terminate the output.
            chars.push_back('\0');
            colors.extend_active(1);

            return { std::move(chars), std::move(colors) };
        }

    private:
        string_encoder_data data{};
    };

    /// @brief Default formatter used for string_encoder.
    /// It represents the testing process in a hierarchical form (group -> tests -> (messages, assertions) -> (test summary, ...) -> group summary).
    class default_string_encoder_formatter {
    public:
        // Make a comment about an event that has happened.
        template <testing_event Event, typename BonusData>
        void comment(const event_info<Event, BonusData>& event, string_encoder_proxy encoder) {
            if constexpr (Event == testing_event::group_start) {
                comment_group_start<Group>(event, encoder);
            } else if constexpr (Event == testing_event::group_end) {
                comment_group_end<Group>(event, encoder);
            }
            else switch (Event) {
            case testing_event::test_start:
                comment_test_start(event, encoder);
                return;
            case testing_event::test_end:
                comment_test_end(event, encoder);
            case testing_event::assertion:
                comment_assertion(event, encoder);
            case testing_event::message:
                comment_message(event, encoder);
            }
        }

    private:
        template <typename Group>
        void comment_group_start(const group_start_info& event, string_encoder_proxy encoder) {
            const console_color active_color = exec.thread.colors.get_active();

            last_group_label_id = exec.thread.colors.reserve(blue_color);
            exec.thread.fmessage("[GROUP] {}\n", Group::name);

            exec.thread.colors.end_reserve(active_color);
        }

        template <typename Group>
        void comment_group_end(const group_end_info& event, string_encoder_proxy encoder) {
            const console_color color = event.result ? green_color : red_color;
            encoder.colors.alter_area(last_group_label_id, color);
            encoder.colors.request(color);
            if (event.result) {
                encoder.fmessage("[TEST-SUMMARY] Test is succesful!\n");
            } else {
                encoder.fmessage("[TEST-SUMMARY] Test was not succesful!\n");
            }
        }

        void comment_test_start(const test_start_info& event, string_encoder_proxy encoder) {
            const console_color active_color = encoder.colors.get_active();

            last_test_label_id = encoder.colors.reserve(blue_color);
            encoder.fmessage("[TEST] {}\n", event.name);

            encoder.colors.end_reserve(active_color);
        }

        void comment_test_end(const test_end_info& event, string_encoder_proxy encoder) {
            const console_color color = event.result ? green_color : red_color;
            encoder.colors.alter_area(last_test_label_id, color);
            encoder.colors.request(color);
            if (event.result) {
                encoder.fmessage("[TEST-SUMMARY] Test is succesful!\n");
            } else {
                encoder.fmessage("[TEST-SUMMARY] Test was not succesful!\n");
            }
        }

    private:
        // Transient data used while building final output, but garbage after the output is ready.
        // Helper variables to store line indentation and some IDs.
        uint16_t line_indentation{};
        uint16_t last_test_label_id{};
        uint16_t last_group_label_id{};
    };
}