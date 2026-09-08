#include "unit_test.hpp"
#include "spdarr.hpp"
#include "platform.hpp"

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
		main_thread.allocate(); // TODO: Pass some sort of configuration here

		for (size_t index{ 1 }; index != threaded_groups + 1; ++index) {
			threading_contexts[index].allocate(); // TODO: Pass some sort of configuration here
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
					threading_contexts[threaded_group].terminate_output();
				}
			};
		}

		for (const group_handler& handler : test_groups | std::views::take(threads_per_group)) {
			handler(main_thread);			
		}
		main_thread.terminate_output();

		for (size_t index{}; index != threaded_groups; ++index) {
			if (!threads[index].joinable()) {
				break;
			}
			threads[index].join();
		}
	}

	void test_manager::display() {
		// TODO: Behavior dependent on color usage, when colors are not used we can just feed whole message at once.
		// Potentially faster version:
		/*for (const auto& context : threading_contexts) {
			if (context.chars.empty()) {
				break;
			}
			std::cout << std::string_view{ context.chars.data(), context.chars.size() };
		}*/

		// Threading contexts now store character buffer "messages" which is the string result of running all tests.
		// We can just output it right now.
		for (const auto& context : threading_contexts) {
			if (context.chars.empty()) {
				break;
			}
			const size_t size = context.areas.size(); // == context.colors.size()

			size_t source_index{};
			for (uint32_t index{}; index != size; ++index) {
				platform_console_color(static_cast<console_color>(context.colors[index]));

				const uint32_t count = context.areas[index];
				platform_console_write(context.chars.data() + source_index, count);
				source_index += static_cast<size_t>(count);
			}
		}

		platform_console_color(white_color);
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