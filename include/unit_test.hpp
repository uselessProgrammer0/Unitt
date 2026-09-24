#pragma once

#include "platform.hpp"
#include "spdarr.hpp"
#include "fixture.hpp"

#include <functional>
#include <concepts>
#include <memory>
#include <array>
#include <vector>
#include <format>
#include <string_view>

// Temporary includes
#include <cassert>
#include <algorithm>
#include <thread>
#include <sstream>
#include <numeric>

namespace unitt
{
    class unit_test;
    class unit_test_group;
    class test_manager;
    class threading_context;
    struct execution;

    using test_function = std::function<void(threading_context&)>;

    enum class test_features {
        none = 0x0,
        measure_execution = 0x01,
    };

    // enum class assertion_type : signed char {
    //     eq,  // ==
    //     neq, // !=
    //     lt,  // <
    //     gt,  // >
    //     le,  // <=
    //     ge,  // >=
    //     throws
    // };

    // enum class assertion_result : signed char {
    //     success = green_color,
    //     failure = red_color,
    //     neutral = blue_color // That's not even really an assertion, we just make it an assertion to universalize behavior.
    // };

    [[nodiscard]] constexpr test_features operator|(test_features left, test_features right) noexcept {
        using Underlying = std::underlying_type_t<test_features>;
        return static_cast<test_features>(static_cast<Underlying>(left) | static_cast<Underlying>(right));
    }

    // enum class testing_event {
    //     group_start,
    //     test_start,
    //     assertion,
    //     message,
    //     test_end,
    //     group_end
    // };

    // template <testing_event, typename Placeholder = void>
    // struct event_info {};

    // template <typename Group>
    // struct event_info<testing_event::group_start, Group> {
    //     constexpr static inline std::string_view name = Group::name;
    // };

    // template <>
    // struct event_info<testing_event::assertion> {
    //     /// @brief Type of the assertion.
    //     assertion_result result{};
    //     assertion_type type{};
    // };

    // template <>
    // struct event_info<testing_event::message> {
    //     /// @brief Message content.
    //     std::string_view message{};
    // };

    // template <>
    // struct event_info<testing_event::test_start> {
    //     /// @brief Name of the test, it references a string with static storage duration.
    //     std::string_view name{};
    // };

    // template <>
    // struct event_info<testing_event::test_end> {
    //     /// @brief Either failure (0), or success (1).
    //     bool result{};
    // };

    // template <>
    // struct event_info<testing_event::group_start> {
    //     /// @brief Either failure (0), or success (1).
    //     /// Failure means that at least one test contained within the group was a failure.
    //     /// Success is the opposite of failure (no tests in the group were a failure).
    //     bool result{};
    // };

    // using group_start_info = event_info<testing_event::group_start>;
    // using test_start_info = event_info<testing_event::test_start>;
    // using assertion_info = event_info<testing_event::assertion>;
    // using message_info = event_info<testing_event::message>;
    // using test_end_info = event_info<testing_event::test_end>;
    // using group_end_info = event_info<testing_event::group_end>;

    // // TODO: Move to a different file.
    // class default_formatter {
    // public:
    //     template <testing_event Event, typename Bonus>
    //     void comment(unitt::execution execi, event_info<Event, Bonus> info) noexcept;

    // private:
    //     uint16_t group_label_id{};
    //     uint16_t test_label_id{};
    // };

    // enum class dispatch_type {
    //     static_dispatch,
    //     dynamic_dispatch
    // };

    // class execution_thread {
    // public:
    //     execution_thread(size_t id) noexcept
    //         : index{ id } {}

    //     [[nodiscard]] bool finished() noexcept {
    //         return false; // TODO: Probably use dispatch to check if we have already executed past the last test for our thread index.
    //     }

    //     size_t test_index{};
    //     size_t index{};
    // };

    // // TODO: Move to a different file.
    // // Informs how test execution is distributed among threads, and where groups are located within tests.
    // class dispatch_info {
    // public:
    //     constexpr dispatch_info(dispatch_type tp) noexcept
    //         : type{ tp }, _static{} {
    //         assert((tp == dispatch_type::static_dispatch) && "For now we only support static dispatch.");
    //     }

    //     /// @brief Get amount of tests that are executed by given thread.
    //     constexpr size_t tests_executed_by(size_t thread) {
    //         switch (type) {
    //             using enum dispatch_type;
    //         case static_dispatch:
    //             return _static.thread_tests[thread];
    //         case dynamic_dispatch:
    //             return 1;
    //         }
    //     }

    //     /// @brief Get index range [first, last) of tests that belong to a given group.
    //     template <typename Group>
    //     constexpr std::pair<uint16_t, uint16_t> group_range() const noexcept {
    //         const size_t id = global_group_id<Group>();
    //         assert((id < group_end_indices.size()) 
    //             && "No tests from given group were ever registered in this dispatch.");
            
    //         const uint16_t end = group_end_indices[id];
    //         const uint16_t count = group_test_count[id];
    //         return { end - count, end };
    //     }

    //     /// @brief Test distribution per group (test count of given group).
    //     template <typename Group>
    //     constexpr uint16_t group_tests() const noexcept {
    //         return group_test_count[global_group_id<Group>()];
    //     }

    //     /// @brief Get number of groups that are being/will be/were dispatched.
    //     constexpr uint16_t group_count() noexcept {
    //         return group_test_count.size();
    //     }

    //     /// @brief Get number of tests that are being/will be/were dispatched.
    //     constexpr uint16_t test_count() noexcept {
    //         return group_end_indices.back();
    //     }

    //     /// @brief Get index range (first, count) of tests that are being/will be/were executed by thread with given ID.
    //     constexpr std::pair<size_t, size_t> thread_range(size_t thread) noexcept {
    //         const size_t offset = std::accumulate(_static.thread_tests.begin(),
    //             std::next(_static.thread_tests.begin(), thread), static_cast<size_t>(0));
    //         return { offset, _static.thread_tests[thread] };
    //     }

    //     // Decide how the threads will execute tests, this information can be later used, because this function does not actually run threads.
    //     constexpr void perform_thread_dispatch(size_t max_threads) {
    //         determine_ranges();

    //         constexpr size_t min_per_thread = 32;
    //         const size_t tests_per_thread = std::max(test_count() / max_threads, min_per_thread);

    //         const size_t total_tests = test_count();
    //         const size_t total_groups = group_count();

    //         thread_count = 1;
    //         for (size_t group{}; group != total_groups; ++group) {
    //             _static.thread_tests[thread_count - 1] += group_test_count[group];
    //             if (_static.thread_tests[thread_count - 1] < tests_per_thread) {
    //                 continue;
    //             }

    //             // TODO: Currently there might be very small amount of tests remaining for next thread (smaller than min_per_thread).
    //             if (const size_t remaining = (total_tests - group_end_indices[group]); (remaining < min_per_thread)) {
    //                 // Add to the preceding thread, because creating new thread would execute too few tests.
    //                 _static.thread_tests[thread_count - 1] += remaining;
    //                 break;
    //             }
                                
    //             ++thread_count;
    //         }
    //     }

    //     template <typename TestClass>
    //     constexpr void register_test() {
    //         const size_t id = global_group_id<typename TestClass::group_type>();
    //         group_test_count.resize(id + 1);
    //         ++group_test_count[id];
    //     }

    // private:
    //     // Initialize group_start_indices with actual indices which indicate start of group ranges.
    //     // We only do that once dispatch is performed, because otherwise we would have to do that everytime a new test is added, which would suck.
    //     constexpr void determine_ranges() {
    //         // This assumes that tests, wherever they are stored, are fractioned by their group ID.
    //         // Only in such case functions such as group_range will work properly.
    //         const size_t groups = group_test_count.size();
    //         group_end_indices.resize(groups);

    //         std::partial_sum(group_test_count.begin(), group_test_count.end(), group_end_indices.begin());
    //     }

    // public:
    //     dispatch_type type{};
    //     size_t thread_count{};

    // private:
    //     std::vector<uint16_t> group_test_count{};
    //     std::vector<uint16_t> group_end_indices{};

    // public:
    //     struct {
    //         std::array<uint16_t, 5> thread_tests;
    //     } _static;
    // };

    class global_id {
    public:
        [[nodiscard]] static size_t next() noexcept {
            return current++;
        }

        [[nodiscard]] static size_t last() noexcept {
            return current;
        }

    private:
        inline static size_t current = 0;
    };

    template <typename Group>
    [[nodiscard]] std::size_t global_group_id() noexcept {
        static const std::size_t id = global_id::next();
        return id;
    }

    // class color_mapper {
    // public:
    //     constexpr color_mapper(console_color initial) noexcept {
    //         colors.push_back(initial);
    //         areas.push_back(0);
    //     }

    //     struct entry {
    //         console_color color{};
    //         uint32_t area{};
    //     };

    // public:
    //     /// @brief Register new color, when active color is same as requested color, nothing happens.
    //     ///   Otherwise, new color entry is created.
    //     ///   In general, after calling this function, color of next message is guaranteed to be the requested one.
    //     ///   You can call this after calling reserve, it will extend the area in case color is same as reserved area's initial color, 
    //     ///     otherwise it will be equivalent to calling end_reserve.
    //     constexpr void request(console_color color) noexcept {
    //         if (colors.back() != color) {
    //             colors.push_back(color);
    //             areas.push_back(0);
    //         }
    //     }

    //     /// @brief Create new color entry, you should only use it if you will change the color of that entry later.
    //     ///   If you want to end the area which will be affected by alter_color_area with returned ID passed, call end_reserve_color.
    //     /// @returns ID value which you can pass to alter_color_area to change the color value of area created by this function.
    //     ///   The returned ID is always different than 0, so you can use 0 as null value.
    //     [[nodiscard]] constexpr uint16_t reserve(console_color color) noexcept {
    //         // Create new entry, even if provided color is same as active (last entry) color,
    //         //   this will allow to change color of that entry later without affecting area of the previous entry.
    //         // In case the color won't be changed, we will lose some speed
    //         //   (two entries for two same colors next to each other, normally that would be just one entry), but that's the job of user to ensure that.
    //         colors.push_back(color);
    //         areas.push_back(0);

    //         return colors.size() - 1; // == areas.size() - 1
    //     }

    //     /// @brief Marks the end of the area which will be affected by calling alter_area with id obtained from preceding reserve call.
    //     ///   You shouldn't call this function without previously calling reserve.
    //     constexpr void end_reserve(console_color color) noexcept {
    //         colors.push_back(color);
    //         areas.push_back(0);
    //     }

    //     /// @brief Get color of last added entry.
    //     [[nodiscard]] constexpr console_color get_active() const noexcept {
    //         return colors.back();
    //     }

    //     /// @brief Change color of previously reserved color area via call to reserve function.
    //     constexpr void alter_area(size_t id, console_color color) noexcept {
    //     #if UNITT_SLOW
    //         // When there are two same colors next to each other, then ideally, we want the new color to be different than the one next to it.
    //         // This not an error though, it's okay logic-wise for this to happen, but we will make more iterations accessing less characters each iteration.
    //         // TODO: We can do a finalizing sweep which would merge same color entries laying next to each other?
    //         if (colors.size() > 1 && id) {
    //             assert((color != colors[id - 1]) // TODO: We don't want to assert, we want to warn.
    //                 && "alter_color_area: promise not fulfilled; set color to a different value than it initially was set via reserve_color.");
    //         }
    //     #endif
    //         colors[id] = color;
    //     }

    //     /// @brief Increase number of characters in the active entry area by chars amount.
    //     constexpr void extend_active(uint32_t chars) noexcept {
    //         areas.back() += chars;
    //     }
       
    //     [[nodiscard]] constexpr size_t size() const noexcept {
    //         return areas.size(); // == colors.size()
    //     }

    //     [[nodiscard]] constexpr entry get_entry(size_t index) const noexcept {
    //         return { colors[index], areas[index] };
    //     }

    // private:
    //     // Every color from colors is active since the index equal to the value of accumulate(*all previous areas values*) for areas[index] characters.
    //     // So given example areas = { 7, 45, 36, 140, 9 } and example colors = { red, blue, red, green, red }, green color is active since 7 + 45 + 36 index, 
    //     //   for 140 characters in the output.
    //     // Also, there can't be two same colors next to each other, because area of "previous" (same as "current") color is just extended in such case, 
    //     //   and size of colors is always equal to size of areas.
    //     // This exists because we want to perform system calls as rarely as possible, while still being able to display various colors.
    //     spdarr<uint32_t, 256> areas{};
    //     spdarr<console_color, 256> colors{}; // TODO: Create something like ioparr, range capable of storing sub-byte integer values per "index", since we will use just 3 colors (so 3 bits) for each index.
    // };

    class threading_context {
    public:
        template <typename... Args>
        void register_assertion(assertion_result type, std::string_view format, const Args&... fmtargs) noexcept {
            colors.request(static_cast<console_color>(type));

            if (type == assertion_result::failure) {
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

        /// @brief Expect that first argument is equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_eq(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_neq(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_gt(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_lt(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_ge(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_le(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            // formatter.comment<testing_event::assertion, void>(, { result, assertion_type::eq });
        }

        /// @brief Format given format string with given arguments and add it to final output.
        template <typename... Args>
        void fmessage(std::string_view format, const Args&... fmtargs) noexcept {
            register_assertion(assertion_result::neutral, format, fmtargs...);
        }

        /// @brief Format given format string with given arguments and add it to final output.
        /// Forces the message to be of given color as well.
        template <typename... Args>
        void fmessage(console_color color, std::string_view format, const Args&... fmtargs) noexcept {
            register_assertion(assertion_result::neutral, format, fmtargs...);
        }

        /// @brief Concatenate given arguments into a single string and add it to final output.
        template <typename... Args>
        std::string_view vmessage(Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
            // TODO: Something like: writer w{ chars }; w.write(std::forward<Args>(args)...);
        }

        /// @brief Concatenate given arguments into a single string and add it to final output.
        /// Forces the message to be of given color as well.
        template <typename... Args>
        std::string_view vmessage(console_color color, Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
            // TODO: Something like: writer w{ chars }; w.write(std::forward<Args>(args)...);
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

        default_formatter formatter{};

        const void* group_fixture{ &no_fixture };
        size_t test_index; // Index of executed test.

        struct {
            signed char value{};
            char symbol{ ' ' };
        } indentation{};

        bool test_result{}; // TODO: Move this to somewhere else, these values are not really part of this context.
        bool group_result{};
    };

    template <typename Group>
    struct group_test_counter {
        inline static uint16_t count{};
    };

    enum class test_use_case {
        get_group_id,
        run_test
    };

    struct execution {
        threading_context& thread;
        const dispatch_info& dispatch;
    };

    class unit_test {
    private:
        template <typename TestClass>
        constexpr unit_test(std::type_identity<TestClass>) noexcept
            : handler{ &use<TestClass> } {}

    public:
        template <typename TestClass>
        [[nodiscard]] constexpr static unit_test from_class() noexcept {
            return { std::type_identity<TestClass>{} };
        }

    public:
        // Get ID of the group that this test belongs to.
        [[nodiscard]] size_t group_id() const noexcept {
            return handler(nullptr, test_use_case::get_group_id);
        }

        void run(const execution& exec) const {
            handler(&exec, test_use_case::run_test);
        }

        template <typename Test>
        static size_t use(const execution* exec, test_use_case use_case) noexcept {
            using Group = typename Test::group_type;
            using GroupFixture = decltype(fixture_creator<Group>::create());

            if (use_case == test_use_case::get_group_id) {
                return global_group_id<Group>();
            }
            if (use_case != test_use_case::run_test) {
                return static_cast<size_t>(-1);
            }
            assert(exec && "exec must be non-zero when use_case == test_use_case::run_test.");
            
            constexpr bool group_has_fixture = !std::is_same_v<GroupFixture, no_fixture_t>;
            const auto [start, end] = group_has_fixture 
                ? exec->dispatch.group_range<Group>() 
                : decltype(exec->dispatch.group_range<Group>()){ 0, 0 };
            if constexpr (group_has_fixture) {
                if (exec->thread.test_index == start) {
                    exec->thread.group_fixture = new GroupFixture{ fixture_creator<Group>::create() };
                }
            }
                      
            exec->thread.indentation.value = 0;
            exec->thread.test_result = true;
            exec->thread.formatter.comment<testing_event::test_start>(*exec,
                event_info<testing_event::test_start>{ Test::name });

            decltype(auto) fixture = fixture_creator<typename Test::test_type>::create();
            Test::run(exec->thread, fixture, *static_cast<const GroupFixture*>(exec->thread.group_fixture));

            if constexpr (group_has_fixture) {
                if (exec->thread.test_index == (end - 1)) {
                    // It's okay to cast that to non-const pointer, because we are the one who created that object.
                    // We don't even have to perform that const_cast anyway, deleting pointer to const is valid C++,
                    //   but I believe it makes it more clear what we do here?
                    delete const_cast<GroupFixture*>(static_cast<const GroupFixture*>(exec->thread.group_fixture));
                    exec->thread.group_fixture = &no_fixture;
                }
            }

            exec->thread.formatter.comment<testing_event::test_end>(*exec,
                event_info<testing_event::test_end>{ exec->thread.test_result });

            ++exec->thread.test_index;
        }
        
        using test_handler = size_t(*)(const execution*, test_use_case);
        test_handler handler{};
    };

    /**
    * @brief Object responsible for running tests.
    */
    class test_manager {
    public:
        void compute();
        void display();
        void compute_and_display();

        template <typename TestClass>
        void register_test() {
            // Partitioned insertion based on TestClass::group_type group id 
            //   (std::sort for now, but we can certainly do better using just a couple of swaps to achieve partitioned ordering).
            tests.push_back(unit_test::from_class<TestClass>());
            std::sort(tests.begin(), tests.end(), [](const unit_test left, const unit_test right) {
                return left.group_id() < right.group_id();
            });

            // That vector also won't be present in the partitioning version probably, that's a temporary solution.
            dispatch.register_test<TestClass>();            
        }

    private:
        void run_tests(threading_context& thread, size_t offset, size_t count);

    private:
        // Maximum number of threads, NOT including main thread!
        constexpr static inline size_t max_thread_count = 4;
        std::array<threading_context, max_thread_count + 1> threading_contexts{};

        dispatch_info dispatch{ dispatch_type::static_dispatch };
        std::vector<unit_test> tests{};        
    };

    inline test_manager tester{};

    // TODO: Internally this treats it as neutral assertion which causes the message to be blue anyway, fix this.
    template <testing_event Event, typename Bonus>
    inline void default_formatter::comment(unitt::execution exec, event_info<Event, Bonus> info) noexcept {
        if constexpr (Event == testing_event::group_start || Event == testing_event::test_start) {
            const console_color active_color = exec.thread.colors.get_active();

            if constexpr (Event == testing_event::group_start) {
                group_label_id = exec.thread.colors.reserve(blue_color);
                exec.thread.fmessage("[GROUP] {}\n", Bonus::name);
            }
            else if constexpr (Event == testing_event::test_start) {
                test_label_id = exec.thread.colors.reserve(blue_color);
                exec.thread.fmessage("[TEST] {}\n", info.name);
            }
            exec.thread.colors.end_reserve(active_color);
        }

        else if constexpr (Event == testing_event::test_end) {
            const console_color color = info.result ? green_color : red_color;
            exec.thread.colors.alter_area(test_label_id, color);
            exec.thread.colors.request(color);
            if (info.result) {
                exec.thread.fmessage("[SUMMARY] Test is succesful!\n");
            } else {
                exec.thread.fmessage("[SUMMARY] Test was not succesful!\n");
            }            
        }

        else if constexpr (Event == testing_event::group_end) {
            exec.thread.colors.alter_area(group_label_id, info.result ? green_color : red_color);
        }
    }

    [[nodiscard]] inline test_manager& global_tester() noexcept {
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