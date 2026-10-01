#pragma once

#include "unit_test.hpp"

#include <vector>
#include <span>

namespace unitt
{
    /// @brief Object storing tests, call "seal" before passing this object to any function.
    class test_storage {
    public:
        /// @brief Add new test.
        template <typename TestClass>
        constexpr void register_test() {
            // Partitioned insertion based on TestClass::group_type group id 
            //   (std::sort for now, but we can certainly do better using just a couple of swaps to achieve partitioned ordering).
            unit_tests.push_back(unit_test::from_class<TestClass>());
            std::sort(unit_tests.begin(), unit_tests.end(), [](const unit_test left, const unit_test right) {
                return left.group_id() < right.group_id();
            });

            const size_t id = global_group_id<typename TestClass::group_type>();
            group_test_count.resize(id + 1);
            ++group_test_count[id];
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

        /// @brief Get number of tests which belong to given group.
        template <typename Group>
        constexpr uint16_t group_tests() const noexcept {
            return group_test_count[global_group_id<Group>()];
        }

        /// @brief Get number of tests which belong to given group.
        constexpr uint16_t group_tests(size_t group_id) const noexcept {
            assert((group_id <= group_test_count.size()) && "Invalid group ID provided for group_test call.");
            return group_test_count[group_id];
        }

        /// @brief Get number of groups that are being/will be/were dispatched.
        constexpr uint16_t group_count() const noexcept {
            return group_test_count.size();
        }

        /// @brief Get number of tests that are being/will be/were dispatched.
        constexpr uint16_t test_count() const noexcept {
            return group_end_indices.back();
        }

        /// @brief Get view to stored unit tests.
        /// You can call this both when sealed and unsealed, but it won't work well with other functions in case the instance is not sealed.
        [[nodiscard]] constexpr std::span<const unit_test> tests() const noexcept {
            return tests;
        }

    public:
        /// @brief Make sure that all data is laid out correctly, call this when you're done adding tests.
        /// Not calling will make execution most likely just crash.
        constexpr void seal() {
            // Initialize group_start_indices with actual indices which indicate start of group ranges.
            // We only do that once dispatch is performed, because otherwise we would have to do that everytime a new test is added, which would suck.
            // This assumes that tests, wherever they are stored, are fractioned by their group ID.
            // Only in such case functions such as group_range will work properly.
            const size_t groups = group_test_count.size();
            group_end_indices.resize(groups);

            std::partial_sum(group_test_count.begin(), group_test_count.end(), group_end_indices.begin());
        }

    private:
        std::vector<uint16_t> group_test_count{};
        std::vector<uint16_t> group_end_indices{};
        std::vector<unit_test> unit_tests{};
    };

    [[nodiscard]] inline test_storage& collected_tests() noexcept {
        static test_storage manager{};
        return manager;
    }

    template <typename TestClass>
    struct test_registrar {
        test_registrar() {
            collected_tests().register_test<TestClass>();
        }
    };
}