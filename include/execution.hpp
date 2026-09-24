#pragma once

namespace unitt
{
    enum class dispatch_type {
        static_dispatch,
        dynamic_dispatch
    };

    class execution_thread {
    public:
        execution_thread(size_t id) noexcept
            : index{ id } {}

        [[nodiscard]] bool finished() noexcept {
            return false; // TODO: Probably use dispatch to check if we have already executed past the last test for our thread index.
        }

        size_t test_index{};
        size_t index{};
    };

    // TODO: Move to a different file.
    // Informs how test execution is distributed among threads, and where groups are located within tests.
    class dispatch_info {
    public:
        constexpr dispatch_info(dispatch_type tp) noexcept
            : type{ tp }, _static{} {
            assert((tp == dispatch_type::static_dispatch) && "For now we only support static dispatch.");
        }

        /// @brief Get amount of tests that are executed by given thread.
        constexpr size_t tests_executed_by(size_t thread) {
            switch (type) {
                using enum dispatch_type;
            case static_dispatch:
                return _static.thread_tests[thread];
            case dynamic_dispatch:
                return 1;
            }
        }

        /// @brief Get index range [first, last) of tests that belong to a given group.
        template <typename Group>
        constexpr std::pair<uint16_t, uint16_t> group_range() const noexcept {
            const size_t id = global_group_id<Group>();
            assert((id < group_end_indices.size()) 
                && "No tests from given group were ever registered in this dispatch.");
            
            const uint16_t end = group_end_indices[id];
            const uint16_t count = group_test_count[id];
            return { end - count, end };
        }

        /// @brief Test distribution per group (test count of given group).
        template <typename Group>
        constexpr uint16_t group_tests() const noexcept {
            return group_test_count[global_group_id<Group>()];
        }

        /// @brief Get number of groups that are being/will be/were dispatched.
        constexpr uint16_t group_count() noexcept {
            return group_test_count.size();
        }

        /// @brief Get number of tests that are being/will be/were dispatched.
        constexpr uint16_t test_count() noexcept {
            return group_end_indices.back();
        }

        /// @brief Get index range (first, count) of tests that are being/will be/were executed by thread with given ID.
        constexpr std::pair<size_t, size_t> thread_range(size_t thread) noexcept {
            const size_t offset = std::accumulate(_static.thread_tests.begin(),
                std::next(_static.thread_tests.begin(), thread), static_cast<size_t>(0));
            return { offset, _static.thread_tests[thread] };
        }

        // Decide how the threads will execute tests, this information can be later used, because this function does not actually run threads.
        constexpr void perform_thread_dispatch(size_t max_threads) {
            determine_ranges();

            constexpr size_t min_per_thread = 32;
            const size_t tests_per_thread = std::max(test_count() / max_threads, min_per_thread);

            const size_t total_tests = test_count();
            const size_t total_groups = group_count();

            thread_count = 1;
            for (size_t group{}; group != total_groups; ++group) {
                _static.thread_tests[thread_count - 1] += group_test_count[group];
                if (_static.thread_tests[thread_count - 1] < tests_per_thread) {
                    continue;
                }

                // TODO: Currently there might be very small amount of tests remaining for next thread (smaller than min_per_thread).
                if (const size_t remaining = (total_tests - group_end_indices[group]); (remaining < min_per_thread)) {
                    // Add to the preceding thread, because creating new thread would execute too few tests.
                    _static.thread_tests[thread_count - 1] += remaining;
                    break;
                }
                                
                ++thread_count;
            }
        }

        template <typename TestClass>
        constexpr void register_test() {
            const size_t id = global_group_id<typename TestClass::group_type>();
            group_test_count.resize(id + 1);
            ++group_test_count[id];
        }

    private:
        // Initialize group_start_indices with actual indices which indicate start of group ranges.
        // We only do that once dispatch is performed, because otherwise we would have to do that everytime a new test is added, which would suck.
        constexpr void determine_ranges() {
            // This assumes that tests, wherever they are stored, are fractioned by their group ID.
            // Only in such case functions such as group_range will work properly.
            const size_t groups = group_test_count.size();
            group_end_indices.resize(groups);

            std::partial_sum(group_test_count.begin(), group_test_count.end(), group_end_indices.begin());
        }

    public:
        dispatch_type type{};
        size_t thread_count{};

    private:
        std::vector<uint16_t> group_test_count{};
        std::vector<uint16_t> group_end_indices{};

    public:
        struct {
            std::array<uint16_t, 5> thread_tests;
        } _static;
    };
}