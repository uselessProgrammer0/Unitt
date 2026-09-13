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

// Temporary includes
#include <source_location>
#include <iostream>
#include <cassert>

namespace unitt
{
    class unit_test;
    class unit_test_group;
    class test_manager;
    class threading_context;

    using test_function = std::function<void(threading_context&)>;

    enum class test_features {
        none = 0x0,
        measure_execution = 0x01,
    };

    struct no_fixture {};

    class unit_test_group {
    public:
        friend class test_manager;
    };

    enum class assertion_type : signed char {
        success = green_color,
        failure = red_color,
        neutral = blue_color // That's not even really an assertion, we just make it an assertion to universalize behavior.
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
        test_summary,
        group_summary
    };

    template <testing_event, typename Placeholder = void>
    struct event_info {};

    template <typename Group>
    struct event_info<testing_event::group, Group> {
        constexpr static inline std::string_view name = Group::name;
    };

    template <>
    struct event_info<testing_event::assertion> {
        /// @brief Type of the assertion.
        assertion_type type{};
    };

    template <>
    struct event_info<testing_event::test> {
        /// @brief Name of the test, it references a string with static storage duration.
        std::string_view name{};
    };

    template <>
    struct event_info<testing_event::test_summary> {
        /// @brief Either failure (0), or success (1).
        bool result{};
    };

    template <>
    struct event_info<testing_event::group_summary> {
        /// @brief Either failure (0), or success (1).
        /// Failure means that at least one test contained within the group was a failure.
        /// Success is the opposite of failure (no tests in the group were a failure).
        bool result{};
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

    class threading_context {
    public:
        void register_test(std::string_view name, test_features features, test_function test_producer) noexcept {
            // This is called inside TEST macro for every test, so we can track indentation here.
            constexpr signed char indent = 4;
            indentation.value = indent;
            fmter.comment<testing_event::test>(*this, event_info<testing_event::test>{ name });
            indentation.value += indent;

            test_result = true;
            test_producer(*this);
            fmter.comment<testing_event::test_summary>(*this, event_info<testing_event::test_summary>{ test_result });
            
            group_result = group_result && test_result;
        }

        template <typename... Args>
        void register_assertion(assertion_type type, std::string_view format, const Args&... fmtargs) noexcept {
            colors.request(static_cast<console_color>(type));

            if (type == assertion_type::failure) {
                test_result = false;
            }

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
            colors.extend_active(1);
        }

        void allocate() {
            chars.reserve(messages_buffer_size);
        }

    public:
        color_mapper colors{ blue_color }; // Blue color is the default color we output in.

        constexpr static inline size_t messages_buffer_size = 4'194'304; // 4MB
        std::vector<char> chars{};

        formatter fmter{};

        struct {
            signed char value{};
            char symbol{ ' ' };
        } indentation{};

        bool test_result{}; // TODO: Move this to somewhere else, these values are not really part of this context.
        bool group_result{};
    };

    // Context methods are "translated" to execute actual context functions.
    class execution_envoy {
        threading_context& context;
    };

    // Context methods are "translate" to do nothing, but since they're called, it means that they would do something if we passed execution_envoy.
    // This way we can find out how many tests will be inside the group for example, even though they can be added dynamically, all we need to do is pass this as context argument.
    struct spy_envoy {
        void register_test(std::string_view, test_features, test_function) noexcept {
            ++test_count;
        }

    public:
        size_t test_count{};
    };

    struct any_group {};
    template <typename Fixture, typename Group = any_group>
    struct fixture {
        static_assert(std::is_default_constructible_v<Fixture>);

        Fixture operator()() noexcept(std::is_nothrow_default_constructible_v<Fixture>) {
            return {};
        }
    };

    template <typename Group>
    struct group_test_counter {
        inline static uint16_t count{};
    };

    enum class group_usecase {
        compute,
        retrieve
    };

    /**
    * @brief Object responsible for running tests.
    */
    class test_manager {
    public:
        void compute();
        void display();
        void compute_and_display();

        /*template <typename Group>
        void register_group(std::source_location srcloc = std::source_location::current()) noexcept {
            Group group{};
            spy_envoy spy{};
            group.collect(spy);

            group_test_counter<Group>::count = spy.test_count;
            test_count += spy.test_count;
            tests.push_back(&use_group<Group>);
        }*/

        template <typename Fixture>
        using test_function = void(*)(threading_context&, Fixture);

        template <typename TestClass>
        void register_test() {
            std::cout << "Registered test!\n";
            // tests.push_back(static_cast<void*>(test));
        }

    private:
        //template <typename Group>
        //static size_t use_group(threading_context& context, group_usecase usecase) { // Calculation only, no output
        //    if (usecase == group_usecase::retrieve) {
        //        return group_test_counter<Group>::count;
        //    }

        //    if (usecase != group_usecase::compute) {
        //        return static_cast<size_t>(-1); // Return value of -1 is invalid for every usecase.
        //    }

        //    context.indentation.value = 0;
        //    context.group_result = true;
        //    context.fmter.comment<testing_event::group, Group>(context, {});

        //    // decltype(auto) fxe = fixture<typename Group::fixture_type>{}();
        //    Group group{};
        //    group.collect(execution_envoy{ context }/*, fxe*/);

        //    context.fmter.comment<testing_event::group_summary>(context, event_info<testing_event::group_summary>{ context.group_result });
        //}

        template <typename Test>
        static size_t use_test(threading_context& context) noexcept {
            Test::run({});
        }

    private:
        // Maximum number of threads, NOT including main thread!
        constexpr static inline size_t max_thread_count = 4;
        std::array<threading_context, max_thread_count + 1> threading_contexts{};

        using test_handler = void(*)(threading_context&, void*);
        std::vector<test_handler> tests{};

        size_t test_count{};
    };

    inline test_manager tester{};

    // TODO: Internally this treats it as neutral assertion which causes the message to be blue anyway, fix this.
    template <testing_event Event, typename Bonus>
    inline void formatter::comment(unitt::threading_context& context, event_info<Event, Bonus> info) noexcept {
        if constexpr (Event == testing_event::group || Event == testing_event::test) {
            const console_color active_color = context.colors.get_active();
            if constexpr (Event == testing_event::group) {
                group_label_id = context.colors.reserve(blue_color);
                context.formatted_message("[GROUP] {}\n", Bonus::name);
            } else if constexpr (Event == testing_event::test) {
                test_label_id = context.colors.reserve(blue_color);
                context.formatted_message("[TEST] {}\n", info.name);
            }
            context.colors.end_reserve(active_color);
        }
        else if constexpr (Event == testing_event::test_summary) {
            const console_color color = info.result ? green_color : red_color;
            context.colors.alter_area(test_label_id, color);
            context.colors.request(color);
            if (info.result) {
                context.formatted_message("[SUMMARY] Test is succesful!\n");
            } else {                
                context.formatted_message("[SUMMARY] Test was not succesful!\n");
            }
            
        }
        else if constexpr (Event == testing_event::group_summary) {
            context.colors.alter_area(group_label_id, info.result ? green_color : red_color);
        }
    }

    [[nodiscard]] inline test_manager global_tester() noexcept {
        static test_manager manager{};
        return manager;
    }

    template <typename TestClass>
    struct test_registrar {
        test_registrar() {
            global_tester().register_test<TestClass>();
        }
    };
}