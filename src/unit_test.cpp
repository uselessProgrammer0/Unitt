#include "unit_test.hpp"
#include "spdarr.hpp"
#include "platform.hpp"

#include <ranges>
#include <thread>
#include <iostream>

namespace unitt
{
	void test_manager::compute() {
		constexpr size_t min_tests_per_thread = 10;

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

//TEST_GROUP(ArithmeticTests, ::unitt::no_fixture)
//	TEST("Addition")
//		const int result{ 15 + 27 };
//		EXPECT_EQ(result, 32);
//	END_TEST
//
//	TEST("Subtraction")
//		const int result{ 50 - 8 };
//		EXPECT_EQ(result, 42);
//	END_TEST
//
//	TEST("Multiplication")
//		const int result{ 6 * 7 };
//		EXPECT_EQ(result, 42);
//	END_TEST
//
//	TEST("Division")
//		const int result{ 84 / 2 };
//		EXPECT_EQ(result, 42);
//	END_TEST
//
//	TEST("Modulo")
//		const int result{ 47 % 5 };
//		EXPECT_EQ(result, 2);
//	END_TEST
//END_TEST_GROUP
//
//TEST_GROUP(ComparisonTests, ::unitt::no_fixture)
//	TEST("Less than")
//		EXPECT_LT(3, 10);
//	END_TEST
//
//	TEST("Greater than")
//		EXPECT_GT(20, 5);
//	END_TEST
//
//	TEST("Equal values")
//		EXPECT_EQ(15, 15);
//	END_TEST
//
//	TEST("Intentional less-than failure")
//		EXPECT_LT(10, 3);
//	END_TEST
//
//	TEST("Intentional equality failure")
//		EXPECT_EQ(7, 8);
//	END_TEST
//END_TEST_GROUP
//
//TEST_GROUP(StringTests, ::unitt::no_fixture)
//	TEST("Empty string")
//		const std::string value{};
//		EXPECT_EQ(value.size(), 0);
//	END_TEST
//
//	TEST("String length")
//		const std::string value{ "hello" };
//		EXPECT_EQ(value.size(), 5);
//	END_TEST
//
//	TEST("String comparison")
//		const std::string value{ "testing" };
//		EXPECT_EQ(value, "testing");
//	END_TEST
//
//	TEST("Different strings should fail")
//		const std::string value{ "foo" };
//		EXPECT_EQ(value, "bar");
//	END_TEST
//
//	TEST("Length should fail")
//		const std::string value{ "abcdef" };
//		EXPECT_EQ(value.size(), 5);
//	END_TEST
//END_TEST_GROUP
//
//TEST_GROUP(BooleanTests, ::unitt::no_fixture)
//	TEST("True expression")
//		const bool result{ 10 > 5 };
//		EXPECT_EQ(result, true);
//	END_TEST
//
//	TEST("False expression")
//		const bool result{ 2 > 8 };
//		EXPECT_EQ(result, false);
//	END_TEST
//
//	TEST("AND works")
//		const bool result{ true && true };
//		EXPECT_EQ(result, true);
//	END_TEST
//
//	TEST("OR works")
//		const bool result{ false || true };
//		EXPECT_EQ(result, true);
//	END_TEST
//
//	TEST("NOT works")
//		const bool result{ !false };
//		EXPECT_EQ(result, true);
//	END_TEST
//END_TEST_GROUP
//
//TEST_GROUP(MixedTests, ::unitt::no_fixture)
//	TEST("Positive number")
//		const int value{ 42 };
//		EXPECT_GT(value, 0);
//	END_TEST
//
//	TEST("Negative number")
//		const int value{ -10 };
//		EXPECT_LT(value, 0);
//	END_TEST
//
//	TEST("Even number")
//		const int value{ 24 };
//		EXPECT_EQ(value % 2, 0);
//	END_TEST
//
//	TEST("Intentional even-number failure")
//		const int value{ 15 };
//		EXPECT_EQ(value % 2, 0);
//	END_TEST
//
//	TEST("Intentional range failure")
//		const int value{ 100 };
//		EXPECT_LT(value, 50);
//	END_TEST
//END_TEST_GROUP

struct IntegerTestsFixture {
	int random_value{ 10 };
};

GLOBAL_TEST(IntegerComparison, IntegerTestsFixture) {
	
}

int main() {
	/*unitt::tester.register_group<ArithmeticTests>();
	unitt::tester.register_group<ComparisonTests>();
	unitt::tester.register_group<StringTests>();
	unitt::tester.register_group<BooleanTests>();
	unitt::tester.register_group<MixedTests>();*/
	unitt::global_tester().compute_and_display();

	return 0;
}