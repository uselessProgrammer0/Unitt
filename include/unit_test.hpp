#pragma once

#include "platform.hpp"
#include "spdarr.hpp"

#include <functional>
#include <concepts>
#include <memory>
#include <array>
#include <vector>
#include <format>
#include <string_view>

#include <cassert>

// Syntactic sugar for declaring test group.
#define TEST_GROUP(group_name, group_fixture) \
class group_name : public ::unitt::unit_test_group { \
public: \
    using fixture_type = group_fixture; \
    constexpr static inline std::string_view name = #group_name; \
    static void collect(::unitt::threading_context& context, const fixture_type& fixture) {

// We need to close group_name::collect, and group_name.
#define END_TEST_GROUP }};

#define STRINGIFY__(x) #x
#define EXPAND__(x) x
#define GLUE_IMPL__(_0, _1) _0##_1
#define GLUE__(_0, _1) GLUE_IMPL__(_0, _1)

#if _MSVC_TRADITIONAL // TODO: The below replacement for __VA_OPT__ doesn't work, fix this (maybe).

    #define VA_CALL__(macro, suffix, ...) EXPAND__(GLUE__(macro, suffix)(__VA_ARGS__))
    #define VA_SELECT__(macro, ...) EXPAND__(VA_CALL__(macro, HAS_ARGS__(__VA_ARGS__), __VA_ARGS__))

    #define HAS_ARGS_IMPL__(_0, _1, _2, _3, _4, _5, N) N
    #define HAS_ARGS__(...) EXPAND__(HAS_ARGS_IMPL__(0, ##__VA_ARGS__, ARGS, ARGS, ARGS, ARGS, ARGS, NOARGS))

    #define COUNT_ARGS__(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N) N
    #define VA_COUNT__(...) EXPAND__(COUNT_ARGS__(0, __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

    #define APPLY_FEATURES__NOARGS() test_features::none
    #define APPLY_FEATURES__ARGS(...) __VA_ARGS__

    // Syntactic sugar for declaring test inside test group.
    #define TEST(test_name, ...) context.register_test(test_name, EXPAND__(VA_SELECT__(APPLY_FEATURES__, __VA_ARGS__)), [&fixture](::unitt::threading_context& context) {

#else

    // Syntactic sugar for declaring test inside test group.
    #define TEST(test_name, ...) context.register_test(test_name, ::unitt::test_features::none __VA_OPT__(| __VA_ARGS__), [&fixture](::unitt::threading_context& context) {

#endif

#define END_TEST });

#define GLOBAL_TEST(non_string_name) void GLUE__(non_string_name, GLUE__(__LINE__, __COUNTER__))__()

#define TEST_MESSAGE(message, ...) context.formatted_message(std::string_view{ message }, __VA_ARGS__)
#define TEST_VMESSAGE(...) context.variadic_message(__VA_ARGS__)

namespace unitt
{
    class unit_test;
    class unit_test_group;
    class test_manager;
    class threading_context;

    using test_function = std::function<void(threading_context&)>;

    struct no_fixture {};

    enum class test_result {
        failure = 0,
        success,
        bypass,
        undefined
    };

    class unit_test_group {
    public:
        friend class test_manager;
    };

    class unit_test {
    public:
        struct expectation {
            std::string_view message{};
            int result{};
        };

    public:
        test_result return_value{};
        std::string_view name{};
    };

    enum class assertion_type : signed char {
        success = green_color,
        failure = red_color,
        neutral = blue_color // That's not even really an assertion, we just make it an assertion to universalize behavior.
    };

    enum class test_features {
        none = 0x0,
        benchmark = 0x01,
    };

    [[nodiscard]] constexpr test_features operator|(test_features left, test_features right) noexcept {
        using Underlying = std::underlying_type_t<test_features>;
        return static_cast<test_features>(static_cast<Underlying>(left) | static_cast<Underlying>(right));
    }

    enum class testing_event {
        group,
        test,
        assertion,
        message,
        test_summary
    };

    template <testing_event, typename Placeholder = void>
    struct event_info {};

    template <typename Group>
    struct event_info<testing_event::group, Group> {
        constexpr static inline std::string_view name = Group::name;
    };

    template <>
    struct event_info<testing_event::assertion> {
        assertion_type type{};
    };

    // TODO: Move to a different file.
    class formatter {
    public:
        template <testing_event Event, typename Bonus>
        void comment(unitt::threading_context& context, event_info<Event, Bonus> info) noexcept;

    private:
        uint16_t group_label_id{};
        uint16_t test_label_id{};
    };

    class threading_context {
    public:
        void register_test(std::string_view name, test_features features, test_function test_producer) noexcept {
            // This is called inside TEST macro for every test, so we can track indentation here.
            constexpr signed char indent = 4;
            indentation.value = indent;

            formatted_message("[TEST] \"{}\"\n", name);

            indentation.value += indent;
            test_producer(*this);
        }

        template <typename... Args>
        void register_assertion(assertion_type type, std::string_view format, const Args&... fmtargs) noexcept {
            register_color(static_cast<console_color>(type));

            const size_t offset = chars.size();
            chars.resize(chars.size() + indentation.value, indentation.symbol);
            if constexpr (sizeof...(Args) == 0) {
                std::copy_n(format.begin(), format.size(), std::back_inserter(chars));
            }
            else {                
                std::vformat_to(std::back_inserter(chars), format, std::make_format_args(fmtargs...));
            }

            areas.back() += chars.size() - offset;
        }

        void register_color(console_color color) noexcept {
            if (colors.back() != color) {
                colors.push_back(color);
                areas.push_back(0);
            }
        }

        /// @brief Create new color entry, you should only use it if you will change the color of that entry later.
        ///   If you want to end the area which will be affected by alter_color_area with returned ID passed, call end_reserve_color.
        /// @returns ID value which you can pass to alter_color_area to change the color value of area created by this function.
        [[nodiscard]] size_t reserve_color(console_color color) noexcept {
            // Create new entry, even if provided color is same as active (last entry) color,
            //   this will allow to change color of that entry later without affecting area of the previous entry.
            // In case the color won't be changed, we will lose some speed
            //   (two entries for two same colors next to each other, normally that would be just one entry), but that's the job of user to ensure that.
            colors.push_back(color);
            areas.push_back(0);

            return colors.size() - 1; // == areas.size() - 1
        }

        void end_reserve_color(console_color color) noexcept {
            colors.push_back(color);
            areas.push_back(0);
        }

        void alter_color_area(size_t id, console_color color) noexcept {
        #if UNITT_SLOW
            // When there are two same colors next to each other, then ideally, we want the new color to be different than the one next to it.
            // This not an error though, it's okay logic-wise for this to happen, but we will make more iterations accessing less characters each iteration.
            // TODO: We can do a finalizing sweep which would merge same color entries laying next to each other?
            if (colors.size() > 1 && id) {
                assert((color != colors[id - 1])
                    && "alter_color_area: promise not fulfilled; set color to a different value than it initially was set via reserve_color.");
            }
        #endif
            colors[id] = color;
        }

        template <typename... Args>
        void formatted_message(std::string_view format, const Args&... fmtargs) noexcept {
            register_assertion(assertion_type::neutral, format, fmtargs...);
        }

        template <typename... Args>
        std::string_view variadic_message(Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
        }

        void terminate_output() {
            chars.push_back('\0');
            ++areas.back();
        }

        void allocate() {
            chars.reserve(messages_buffer_size);
            colors.push_back(blue_color);
            areas.push_back(0);
        }

    public:
        constexpr static inline size_t messages_buffer_size = 4'194'304; // 4MB
        std::vector<char> chars{};

        // Every color from colors is active since the index equal to the value of accumulate(*all previous areas values*) for areas[index] characters.
        // So given example areas = { 7, 45, 36, 140, 9 } and example colors = { red, blue, red, green, red }, green color is active since 7 + 45 + 36 index, 
        //   for 140 characters in the output.
        // Also, there can't be two same colors next to each other, because area of "previous" (same as "current") color is just extended in such case, 
        //   and size of colors is always equal to size of areas.
        // This exists because we want to perform system calls as rarely as possible, while still being able to display various colors.
        spdarr<uint32_t, 256> areas{};
        spdarr<console_color, 256> colors{}; // TODO: Create something like ioparr, range capable of storing sub-byte integer values per "index", since we will use just 3 colors (so 3 bits) for each index.

    public:
        struct {
            signed char value{};
            char symbol{ ' ' };
        } indentation{};
    };

    struct any_group {};
    template <typename Fixture, typename Group = any_group>
    struct fixture {
        static_assert(std::is_default_constructible_v<Fixture>);

        Fixture operator()() noexcept(std::is_nothrow_default_constructible_v<Fixture>) {
            return {};
        }
    };

    /**
    * @brief Object responsible for running tests.
    */
    class test_manager {
    public:
        void compute();
        void display();
        void compute_and_display();

        template <typename Group>
        void register_group() noexcept {
            test_groups.push_back(&compute_group<Group>);
        }

    private:
        template <typename Group>
        static void compute_group(threading_context& context) { // Calculation only, no output
            Group group{};

            formatter{}.comment<testing_event::group, Group>(context, {});

            context.indentation.value = 0;
            decltype(auto) fxe = fixture<typename Group::fixture_type>{}();
            group.collect(context, fxe);
        }

    private:
        // Maximum number of threads, NOT including main thread!
        constexpr static inline size_t max_thread_count = 4;
        std::array<threading_context, max_thread_count + 1> threading_contexts{};

        using group_handler = void(*)(threading_context&);
        std::vector<group_handler> test_groups{};
    };

    inline test_manager tester{};

    template <testing_event Event, typename Bonus>
    inline void formatter::comment(unitt::threading_context& context, event_info<Event, Bonus> info) noexcept {
        if constexpr (Event == testing_event::group) {
            group_label_id = context.reserve_color(blue_color);
            context.formatted_message("[GROUP] {}\n", Bonus::name);
            context.end_reserve_color();
        } else if constexpr (Event == testing_event::test) {
            test_label_id = context.reserve_color(blue_color);
            context.formatted_message("{}\n", info.type);
            context.end_reserve_color();
        } else if constexpr (Event == testing_event::test_summary) {
            context.alter_color_area(test_label_id, /* Result of running latest test converted to color */);
        } else if constexpr (Event == testing_event::group_summary) {
            context.alter_color_area(group_label_id, /* Result of collectively running tests (conjunction) converted to color */);
        }
    }
}