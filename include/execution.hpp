#pragma once

#include "register.hpp"
#include "encoder.hpp"

#include <array>
#include <thread>
#include <ranges>
#include <type_traits>

#include <cassert>

namespace unitt
{
    template <typename...>
    struct type_list {};

    struct global_encoders {};
    template <typename Tag>
    struct encoders {
        using type = type_list<string_encoder<>>;
    };

    template <typename... Encoders>
    constexpr auto _encoders_build_types(type_list<Encoders...>) noexcept {
        return ::std::type_identity<std::tuple<decltype(std::declval<Encoders&>().build())...>>{};
    }

    template <typename... Encoders>
    constexpr auto _encoders_as_parameters_types(type_list<Encoders...>) noexcept {
        return ::std::type_identity<std::tuple<Encoders*...>>{};
    }

    template <typename ForceLazyDoNotPass = void>
    using encoders_build_types = typename decltype(_encoders_build_types(encoders<global_encoders>::type{}))::type;

    template <typename ForceLazyDoNotPass = void>
    using encoders_as_parameters_types = typename decltype(_encoders_as_parameters_types(encoders<global_encoders>::type{}))::type;

    enum class dispatch_type {
        static_dispatch,
        dynamic_dispatch
    };

    class thread_dispatch {
    public:
        constexpr thread_dispatch(dispatch_type tp) noexcept
            : type{ tp } {
            assert((tp == dispatch_type::static_dispatch) && "For now we only support static dispatch.");
        }
    
    public:
        /// @brief Get amount of tests that are executed by given thread.
        constexpr size_t executed_by(const execution_thread& thread) const noexcept {
            switch (type) {                
            case dispatch_type::static_dispatch:
                return thread_tests[thread.get_id()];
            case dispatch_type::dynamic_dispatch:
                return 1;
            }
        }

        /// @brief Get index range (first, count) of tests that are being/will be/were executed by thread with given ID.
        constexpr std::pair<size_t, size_t> thread_range(const execution_thread& thread) noexcept {
            const size_t offset = std::accumulate(thread_tests.begin(),
                std::next(thread_tests.begin(), thread.get_id()), static_cast<size_t>(0));
            return { offset, thread_tests[thread.get_id()] };
        }

        /// @brief Decide how the threads will execute tests, and store this information internally, this function does not launch threads.
        /// Calling this function after it was already called before will overwrite previous dispatch information.
        [[nodiscard]] static thread_dispatch perform(const test_storage& tests, size_t max_threads) {
            thread_dispatch dispatch{ dispatch_type::static_dispatch };

            const size_t total_tests = tests.test_count();
            const size_t total_groups = tests.group_count();
            
            constexpr size_t min_per_thread = 32;
            const size_t tests_per_thread = std::max(total_tests / max_threads, min_per_thread);

            dispatch.thread_count = 1;
            size_t dispatched_tests{};

            for (size_t group{}; group != total_groups; ++group) {
                dispatch.thread_tests[dispatch.thread_count - 1] += tests.group_tests(group);
                dispatched_tests += tests.group_tests(group);

                if (dispatch.thread_tests[dispatch.thread_count - 1] < tests_per_thread) {
                    continue;
                }

                if (const size_t remaining = (total_tests - dispatched_tests); (remaining < min_per_thread)) {
                    // Add to the preceding thread, because creating new thread would execute too few tests.
                    dispatch.thread_tests[dispatch.thread_count - 1] += remaining;
                    break;
                }

                ++dispatch.thread_count;
            }

            return dispatch;
        }

    public:
        dispatch_type type{};

        /// @brief Number of threads that tests are dispatched to run on, always at least one.
        size_t thread_count{};

    public:
        std::array<uint16_t, 5> thread_tests{};
    };

    struct execution_info;

    template <typename... Encoders>
    class test_interface {
        /// @brief Expect that first argument is equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_eq(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
            encoder.encode(execution_ref, assertion_info{ result, assertion_type::eq });
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_neq(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_gt(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_lt(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_ge(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
        }

        /// @brief Expect that first argument is not equal to second argument.
        /// Formatter decides what is the output based on the comparison result.
        template <typename First, typename Second>
        void expect_le(const First& first, const Second& second) {
            const assertion_result result = (first == second) ? assertion_result::success : assertion_result::failure;
        }

        /// @brief Format given format string with given arguments and add it to final output.
        /// The requested color used for that message will be blue.
        template <typename... Args>
        void fmessage(std::string_view format, const Args&... fmtargs) noexcept {
            encoder.encode(execution_ref, message_info{  });
        }

        /// @brief Format given format string with given arguments and add it to final output.
        /// Implies that the message should be of given color as well, but whether that request will be respected depends on the encoders used.
        template <typename... Args>
        void fmessage(console_color color, std::string_view format, const Args&... fmtargs) noexcept {
            // TODO: Implement, copy code from unit_test.hpp
        }

        /// @brief Concatenate given arguments into a single string and add it to final output.
        template <typename... Args>
        std::string_view vmessage(Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
            // TODO: Something like: writer w{ chars }; w.write(std::forward<Args>(args)...);
        }

        /// @brief Concatenate given arguments into a single string and add it to final output.
        /// Implies that the message should be of given color as well, but whether that request will be respected depends on the encoder/formatter used.
        template <typename... Args>
        std::string_view vmessage(console_color color, Args&&... args) noexcept {
            // TODO: For now we don't have any reliable writer, create it.
            // TODO: Something like: writer w{ chars }; w.write(std::forward<Args>(args)...);
        }

    private:
        void emit_assertion(assertion_result result, assertion_type type) noexcept {
            if (result == assertion_result::failure) {
                test_result = false;
            }
            encoder.encode(execution_ref, assertion_info{ result, type });
        }

    private:
        execution_info& execution_ref;
        bool test_result{}; // Result of running the test, it is determined by what the expectation results were.
    };

    class execution_thread {
    public:
        execution_thread(size_t id) noexcept
            : self_id{ id } {}

        /// @brief Check if this thread has already finished executing all tests that it was supposed to execute.
        /// @param dispatch Dispatch according to which the check will be performed.
        [[nodiscard]] bool finished(const thread_dispatch& dispatch) noexcept {
            return test_index == dispatch.executed_by(self_id);
        }

        /// @brief Run single unit test.
        template <typename Test>
        void run_test(const execution_info& execution);

        [[nodiscard]] size_t get_id() const noexcept {
            return self_id;
        }

    public:
        string_encoder<> encoder{}; // Encoder is stored per thread.

    private:
        const void* group_fixture{ &no_fixture };
        size_t test_index{}; // Index of executed test.
        size_t self_id{}; // Index of this thread, it's ID.
        bool group_result{}; // Result of running last group.
    };

    struct execution_info {
        std::vector<execution_thread> threads{};
        thread_dispatch dispatch{ dispatch_type::static_dispatch };
    };

    template <typename Test>
    void execution_thread::run_test(const execution_info& execution) {
        using Group = typename Test::group_type;
        using GroupFixture = decltype(fixture_creator<Group>::create());

        constexpr bool group_has_fixture = !std::is_same_v<GroupFixture, no_fixture_t>;
        const auto [start, end] = group_has_fixture
            ? execution.dispatch.group_range<Group>() 
            : decltype(execution.dispatch.group_range<Group>()){ 0, 0 };
        if constexpr (group_has_fixture) {
            if (test_index == start) {
                group_fixture = new GroupFixture{ fixture_creator<Group>::create() };
            }
        }

        encoder.encode<testing_event::test_start>();

        decltype(auto) fixture = fixture_creator<typename Test::test_type>::create();
        test_interface test_messenger{};
        Test::run(test_messenger, fixture, *static_cast<const GroupFixture*>(group_fixture));

        if (test_index == (end - 1)) {
            encoder.encode<testing_event::group_end>(execution, );
            group_result = false;

            if constexpr (group_has_fixture) {
                // It's okay to cast that to non-const pointer, because we are the one who created that object.
                // We don't even have to perform that const_cast anyway, deleting pointer to const is valid C++,
                //   but I believe it makes it more clear what we do here?
                delete const_cast<GroupFixture*>(static_cast<const GroupFixture*>(group_fixture));
                group_fixture = &no_fixture;
            }
        }

        encoder.encode(execution, test_end_info{ test_messenger.test_result });

        ++test_index;
    }

    [[nodiscard]] encoders_build_types<>
                run_tests(const test_storage& tests, encoders_as_parameters_types<> encoder_refs) {
        execution_info execution{};

		execution.dispatch = thread_dispatch::perform(tests, 4);
        execution.threads.resize(execution.dispatch.thread_count);

		std::thread threads[16] {};

		for (size_t index{ 1 }; index != execution.dispatch.thread_count; ++index) {
            execution_thread& thread = execution.threads.emplace_back(index);
            thread.encoder.initialize();

			threads[index - 1] = std::thread {
				[&, index] {                    
					const auto [offset, count] = execution.dispatch.thread_range(thread); // TODO: Make thread_range take some sort of virtual thread as parameter and read it's ID in the function instead of passing index.
                    
                    for (const unit_test& test : tests.tests()
                        | std::views::drop(offset)
                        | std::views::take(count)) {
                        test.run(execution, thread);
                    }
				}
			};
		}

        execution_thread& main_thread = execution.threads[0];
        main_thread.encoder.initialize();
		for (const unit_test& test : tests.tests()
            | std::views::take(execution.dispatch.executed_by(main_thread)) {
            test.run(execution, main_thread);
        }

		for (size_t thread{ 1 }; thread != execution.dispatch.thread_count; ++thread) {
			if (threads[thread - 1].joinable()) {
				threads[thread - 1].join();
			}
		}
    }

    // TODO: Move to a different file.
    // Informs how test execution is distributed among threads, and where groups are located within tests.
    // class dispatch_info {
    // public:
        // constexpr dispatch_info(dispatch_type tp) noexcept
        //     : type{ tp }, _static{} {
        //     assert((tp == dispatch_type::static_dispatch) && "For now we only support static dispatch.");
        // }

        // /// @brief Get amount of tests that are executed by given thread.
        // constexpr size_t tests_executed_by(size_t thread) {
        //     switch (type) {
        //         using enum dispatch_type;
        //     case static_dispatch:
        //         return _static.thread_tests[thread];
        //     case dynamic_dispatch:
        //         return 1;
        //     }
        // }

        // /// @brief Get index range [first, last) of tests that belong to a given group.
        // template <typename Group>
        // constexpr std::pair<uint16_t, uint16_t> group_range() const noexcept {
        //     const size_t id = global_group_id<Group>();
        //     assert((id < group_end_indices.size()) 
        //         && "No tests from given group were ever registered in this dispatch.");
            
        //     const uint16_t end = group_end_indices[id];
        //     const uint16_t count = group_test_count[id];
        //     return { end - count, end };
        // }

        // /// @brief Test distribution per group (test count of given group).
        // template <typename Group>
        // constexpr uint16_t group_tests() const noexcept {
        //     return group_test_count[global_group_id<Group>()];
        // }

        // /// @brief Get number of groups that are being/will be/were dispatched.
        // constexpr uint16_t group_count() noexcept {
        //     return group_test_count.size();
        // }

        // /// @brief Get number of tests that are being/will be/were dispatched.
        // constexpr uint16_t test_count() noexcept {
        //     return group_end_indices.back();
        // }

        // /// @brief Get index range (first, count) of tests that are being/will be/were executed by thread with given ID.
        // constexpr std::pair<size_t, size_t> thread_range(size_t thread) noexcept {
        //     const size_t offset = std::accumulate(_static.thread_tests.begin(),
        //         std::next(_static.thread_tests.begin(), thread), static_cast<size_t>(0));
        //     return { offset, _static.thread_tests[thread] };
        // }

        // // Decide how the threads will execute tests, this information can be later used, because this function does not actually run threads.
        // constexpr void perform_thread_dispatch(size_t max_threads) {
        //     determine_ranges();

        //     constexpr size_t min_per_thread = 32;
        //     const size_t tests_per_thread = std::max(test_count() / max_threads, min_per_thread);

        //     const size_t total_tests = test_count();
        //     const size_t total_groups = group_count();

        //     thread_count = 1;
        //     for (size_t group{}; group != total_groups; ++group) {
        //         _static.thread_tests[thread_count - 1] += group_test_count[group];
        //         if (_static.thread_tests[thread_count - 1] < tests_per_thread) {
        //             continue;
        //         }

        //         // TODO: Currently there might be very small amount of tests remaining for next thread (smaller than min_per_thread).
        //         if (const size_t remaining = (total_tests - group_end_indices[group]); (remaining < min_per_thread)) {
        //             // Add to the preceding thread, because creating new thread would execute too few tests.
        //             _static.thread_tests[thread_count - 1] += remaining;
        //             break;
        //         }

        //         ++thread_count;
        //     }
        // }

        // template <typename TestClass>
        // constexpr void register_test() {
        //     const size_t id = global_group_id<typename TestClass::group_type>();
        //     group_test_count.resize(id + 1);
        //     ++group_test_count[id];
        // }

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
}