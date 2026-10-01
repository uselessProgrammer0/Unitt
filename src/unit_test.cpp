#include "unit_test.hpp"
#include "spdarr.hpp"
#include "platform/console.hpp"

#include <ranges>
#include <thread>
#include <iostream>
#include <span>
#include <numeric>

namespace unitt
{
//	void test_manager::run_tests(threading_context& thread, size_t offset, size_t count) {
//		const execution exec{ thread, dispatch };
//		for (const unit_test test : tests
//			| std::views::drop(offset)
//			| std::views::take(count)) {
//			test.run(exec);
//		}
//		thread.terminate_output();
//	}
//
//	void test_manager::compute() {
//		// This requires main thread to be passed as a separate thread too.
//		dispatch.perform_thread_dispatch(max_thread_count + 1);
//
//		std::thread threads[max_thread_count] {};
//		for (size_t index{ 1 }; index != dispatch.thread_count; ++index) {
//			threading_contexts[index].allocate();
//
//			threads[index - 1] = std::thread {
//				[this, index] {
//					threading_context& thread = threading_contexts[index];
//					const auto [offset, count] = dispatch.thread_range(index); // TODO: Make thread_range take some sort of virtual thread as parameter and read it's ID in the function instead of passing index.
//					run_tests(thread, offset, count);
//				}
//			};
//		}
//
//		threading_context& main = threading_contexts[0];
//		main.allocate();
//		run_tests(main, 0, dispatch._static.thread_tests[0]);
//
//		for (size_t thread{ 1 }; thread != dispatch.thread_count; ++thread) {
//			if (threads[thread - 1].joinable()) {
//				threads[thread - 1].join();
//			}
//		}
//	}
//
//	void test_manager::display() {
//		// Threading contexts now store character buffer "messages" which is the string result of running all tests.
//		// We can just output it right now.
//		for (const auto& context : threading_contexts) {
//			if (context.chars.empty()) {
//				break;
//			}
//			const size_t size = context.colors.size();
//
//			size_t source_index{};
//			for (uint32_t index{}; index != size; ++index) {
//				const color_mapper::entry entry = context.colors.get_entry(index);
//				platform_console_color(entry.color);
//				
//				platform_console_write(context.chars.data() + source_index, entry.area);
//				source_index += static_cast<size_t>(entry.area);
//			}
//		}
//
//		platform_console_color(white_color);
//	}
//
//	void test_manager::compute_and_display() {
//		compute();
//		display();
//	}
//}

#include "test_macros.hpp"

//struct MathTestsGroupFixture {
//	MathTestsGroupFixture() {
//		std::cout << "MathTestsGroupFixture group fixture was initialized.\n";
//	}
//
//	~MathTestsGroupFixture() {
//		std::cout << "MathTestsGroupFixture group fixture was destroyed.\n";
//	}
//};
//
//struct MathTestsTestFixture {
//	MathTestsTestFixture() {
//		std::cout << "MathTestsTestFixture group fixture was initialized.\n";
//	}
//
//	~MathTestsTestFixture() {
//		std::cout << "MathTestsTestFixture group fixture was destroyed.\n";
//	}
//};
//
//TEST(MathTests, IntegerComparisonWorks) {
//	TEST_MESSAGE("This test will check if two integers compare equal for obvious cases.\n");
//	EXPECT_EQ(10, 10);
//}
//
//TEST(MathTests, IntegerSubtractionWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerMultiplicationWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerAdditionWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerDivisionWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerModuloWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerShrWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(MathTests, IntegerShlWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(VectorTests, DotWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(VectorTests, AdditionWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(VectorTests, SubtractionWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(VectorTests, MultiplicationWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_EQ(20 - 10, 10);
//}
//
//TEST(VectorTests, VectorNormalizationWorks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_GT(0, 0);
//}
//
//TEST(StringTests, LongDescriptionOfWhatTheTestActuallyChecks) {
//	TEST_MESSAGE("Whatever test message just to test if tester works.\n");
//	EXPECT_GT(0, 0);
//}
//
//ENCODERS(::unitt::string_encoder<>);
//
//template <typename Identifier>
//struct Notifier {
//	Notifier(Identifier id) noexcept 
//		: identifier{ std::move(id) } {
//		std::cout << "Notifier with identifier " << identifier << " was created.\n";
//	}
//
//	~Notifier() {
//		std::cout << "Notifier with identifier " << identifier << " was destroyed.\n";
//	}
//
//	Identifier identifier;
//};
//
//FIXTURE(Group::VectorTests) {
//	return Notifier{ "\"Group::VectorTests\"" };
//}

#include "register.hpp"

int main() {
	unitt::test_storage tests{};

	unitt::string_encoder encoder{};
	unitt::run_tests(tests, &encoder);
	return 0;
}