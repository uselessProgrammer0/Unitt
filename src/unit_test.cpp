#include "unit_test.hpp"
#include "sdarr.hpp"
#include "pdarr.hpp"

#include <ranges>
#include <thread>
#include <iostream>

namespace unitt
{
	void test_manager::compute() {
		constexpr size_t min_groups_per_thread = 10;

		// Number of additional threads ran, main thread is always ran
		const size_t threaded_groups = std::min(test_groups.size() / min_groups_per_thread, max_thread_count);
		const size_t threads_per_group = threaded_groups 
			? test_groups.size() / threaded_groups
			: test_groups.size();

		threading_context& main_thread = threading_contexts.front();
		main_thread.allocate();

		for (size_t index{ 1 }; index != threaded_groups + 1; ++index) {
			threading_contexts[index].allocate();
		}

		std::array<std::thread, max_thread_count> threads{};
		for (size_t threaded_group{}; threaded_group != threaded_groups; ++threaded_group) {
			threads[threaded_group] = std::thread{
				[this, threaded_group, threads_per_group] {
					for (const group_handler& handler : test_groups
							| std::views::drop(threaded_group * threads_per_group)
							| std::views::take(threads_per_group)) {
						handler(threading_contexts[threaded_group]);
					}
					threading_contexts[threaded_group].chars.push_back('\0');
				}
			};
		}

		for (const group_handler& handler : test_groups | std::views::take(threads_per_group)) {
			handler(main_thread);			
		}
		main_thread.chars.push_back('\0');

		for (size_t index{}; index != threaded_groups; ++index) {
			if (!threads[index].joinable()) {
				break;
			}
			threads[index].join();
		}
	}

	void test_manager::display() {
		// Threading contexts now store character buffer "messages" which is the string result of running all tests.
		// We can just output it right now.
		for (const auto& context : threading_contexts) {
			if (context.chars.empty()) {
				break;
			}
			std::cout << std::string_view{ context.chars.data(), context.chars.size() };
		}
	}

	void test_manager::compute_and_display() {
		compute();
		display();
	}
}

#include "expect.hpp"

struct IntegerTestsFixture {
	int random_value{ 10 };
};

TEST_GROUP(IntegerTests, IntegerTestsFixture)
	TEST("Comparison works", ::unitt::test_features::benchmark)
		EXPECT_EQ(fixture.random_value, 10);
	END_TEST

	TEST("Addition works")
		const int x{}, y{};
		const int sum{ x + y };
		EXPECT_EQ(sum, 0);
	END_TEST

	TEST("Subtraction works")
		const int x{}, y{};
		const int diff{ x - y };
		EXPECT_EQ(diff, 1);
	END_TEST
END_TEST_GROUP

int main() {
	unitt::tester.register_group<IntegerTests>();
	unitt::tester.compute_and_display();

	return 0;
}