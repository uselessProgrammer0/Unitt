#pragma once

// #include "UrsaUtil/Log/log.h"
// #include "UrsaCore/Core/application_clock.h"

#include "sdarr.hpp"

#include <functional>
#include <concepts>
#include <memory>
#include <array>
#include <vector>
#include <format>
#include <string_view>

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

    enum class assertion_type {
        success = 0,
        failure,
        neutral // That's not even really an assertion, we just make it an assertion to not be forced to create another buffer for user messages.
    };

    enum class test_features {
        none = 0x0,
        benchmark = 0x01,
    };

    [[nodiscard]] constexpr test_features operator|(test_features left, test_features right) noexcept {
        using Underlying = std::underlying_type_t<test_features>;
        return static_cast<test_features>(static_cast<Underlying>(left) | static_cast<Underlying>(right));
    }

    enum output_char_trait : signed char {
        red_color = 0x01,
        green_color = 0x02,
        blue_color = 0x03,
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
            if constexpr (sizeof...(Args) == 0) {
                for (size_t rep{ static_cast<size_t>(indentation.value) }; rep-- > 0;) chars.push_back(indentation.symbol);
                std::copy_n(format.begin(), format.size(), std::back_inserter(chars));
                return;
            }

            const size_t offset = chars.size();
            chars.resize(chars.size() + indentation.value, indentation.symbol);
            std::vformat_to(std::back_inserter(chars), format, std::make_format_args(fmtargs...));
        }

        template <typename... Args>
        void formatted_message(std::string_view format, const Args&... fmtargs) noexcept {
            register_assertion(assertion_type::neutral, format, fmtargs...);
        }

        template <typename... Args>
        std::string_view variadic_message(Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
        }

        void allocate(test_manager& manager) {
            chars.reserve(messages_buffer_size);
            if (!manager.no_output_colors) {
                traits.reserve(messages_buffer_size);
            }
        }

    public:
        constexpr static inline size_t messages_buffer_size = 4'194'304; // 4MB
        std::vector<char> chars{};
        std::vector<signed char> traits{};

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

            context.formatted_message(std::string_view{ "[GROUP] \"{}\"\n" }, Group::name);

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
}