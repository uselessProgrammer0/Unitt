#include "unit_test.hpp"
#include "spdarr.hpp"
#include "platform.hpp"

#include <ranges>
#include <thread>
#include <iostream>
#include <span>
#include <numeric>

namespace unitt
{
	void test_manager::run_tests(threading_context& thread, size_t offset, size_t count) {
		for (const auto handler : tests
			| std::views::drop(offset)
			| std::views::take(count)) {
			handler(&thread, test_use_case::run_test);
		}
		thread.terminate_output();
	}

	void test_manager::compute() {
		constexpr size_t min_per_thread = 4;
		const bool additional_threads = tests.size() / min_per_thread;
		const size_t tests_per_thread = std::max(tests.size() / max_thread_count, min_per_thread);
		
		if (tests.empty()) {
			return;
		}

		dispatch_info dispatch{ dispatch_type::static_dispatch };

		threading_contexts.front().allocate(); // We always use main thread, so we can always allocate it.
		std::array<std::thread, max_thread_count> threads{};
		size_t thread_count{};

		size_t test_count{};
		size_t main_thread_tests{};

		for (size_t group{}; group != groups_test_count.size(); ++group) {
			test_count += groups_test_count[group];
			if (test_count < tests_per_thread) {
				continue;
			}
			
			threading_contexts[thread_count].allocate();

			if (thread_count == 0) { // We'll launch on the main thread later, we can't do that now, as we'd block this loop.
				main_thread_tests = test_count;
			}
			else { // TODO: Currently there might be very small amount of tests remaining for next thread (smaller than min_per_thread).
				threads[thread_count - 1] = std::thread { // We might consider checking and handling that somehow.
					[this, test_count, group, thread_count] {
						const size_t offset = std::accumulate(groups_test_count.begin(),
							std::next(groups_test_count.begin(), group), 0);
						run_tests(threading_contexts[thread_count], offset, test_count);
					}
				};
			}

			dispatch._static.thread_tests[thread_count] = test_count;
			test_count = 0;
			++thread_count;
		}

		// NOTE: There are (thread_count - 1) running threads at this point, and the remaining one thread will be run if there are any tests left for it.		

		if (thread_count == 0) {
			std::printf("Running %zu groups (%zu tests) on 1 thread (average +-%zu tests per thread).\n",
				groups_test_count.size(), tests.size(), tests.size());

			// Means that we run on main thread only.
			run_tests(threading_contexts.front(), 0, tests.size());
		}

		if (thread_count > 0) {
			if (test_count) { // There are tests remaining, but we didn't put them on thread yet.
				threads[thread_count - 1] = std::thread {
					[this, test_count, thread_count] {
						run_tests(threading_contexts[thread_count], tests.size() - test_count, test_count);
					}
				};
			}
			else {
				--thread_count; // We didn't run it eventually...
			}

			dispatch.thread_count = thread_count;
			const size_t tests_per_thread = std::accumulate(dispatch._static.thread_tests.data(),
				dispatch._static.thread_tests.data() + dispatch.thread_count, static_cast<size_t>(0)) / (thread_count + 1);
			std::printf("Running %zu groups (%zu tests) on %zu threads (average +-%zu tests per thread).\n",
				groups_test_count.size(), tests.size(), thread_count + 1, tests_per_thread);

			// We run on main thread and some other thread(s).
			run_tests(threading_contexts.front(), 0, main_thread_tests);

			// Main thread is not stored as std::thread, thus the preincrement.
			for (; thread_count--;) {
				if (!threads[thread_count].joinable()) {
					continue;
				}
				threads[thread_count].join();
			}
		}
	}

	void test_manager::display() {
		// Threading contexts now store character buffer "messages" which is the string result of running all tests.
		// We can just output it right now.
		for (const auto& context : threading_contexts) {
			if (context.chars.empty()) {
				break;
			}
			const size_t size = context.colors.size();

			size_t source_index{};
			for (uint32_t index{}; index != size; ++index) {
				const color_mapper::entry entry = context.colors.get_entry(index);
				platform_console_color(entry.color);
				
				platform_console_write(context.chars.data() + source_index, entry.area);
				source_index += static_cast<size_t>(entry.area);
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
#include "test_macros.hpp"

struct MathTests {
	MathTests() {
		std::cout << "MathTest group context was initialized.\n";
	}

	~MathTests() {
		std::cout << "MathTest group context was destroyed.\n";
	}
};

TEST(MathTests, IntegerComparisonWorks) {
	TEST_MESSAGE("This test will check if two integers compare equal for obvious cases.\n");
	EXPECT_EQ(10, 10);
}

TEST(MathTests, IntegerSubtractionWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerMultiplicationWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerAdditionWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerDivisionWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerModuloWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerShrWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(MathTests, IntegerShlWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(VectorTests, DotWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(VectorTests, AdditionWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(VectorTests, SubtractionWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(VectorTests, MultiplicationWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_EQ(20 - 10, 10);
}

TEST(VectorTests, VectorNormalizationWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_GT(0, 0);
}

TEST(StringTests, ComparisonWorks) {
	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
	EXPECT_GT(0, 0);
}

int main() {
	unitt::global_tester().compute_and_display();

	return 0;
}