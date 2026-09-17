#pragma once

// Assert any expression.
// Assertion prevents all expectations and other assertions from happening if it fails (it "cuts" the test).
// Failure does not do anything besides what described above.
#define ASSERT

#define TOSTRING(anything) #anything

// Assertion involving operator.
#define ASSERT_OP(x, y, op) \
do { \
    if (!((x) op (y))) { \
        ::ursa::log::fatal(::ursa::util::Tester._Indentation_channel, "[FAIL] Expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is not the case [" \
        TOSTRING(x) " = ") << x << ", " TOSTRING(y) " = " << y << "]."; \
        test.return_value = ::ursa::util::Test_Result::FAILURE; \
        return test_obj; \
    } \
    ::ursa::log::info(::ursa::util::Tester._Indentation_channel, "[SUCCESS] expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is not the case [" \
        TOSTRING(x) " = ", x) << x << ", " TOSTRING(y) " = " << y << "]."; \
} while (false)

// Assert that x == y.
#define ASSERT_EQ(x, y)  ASSERT_OP(x, y, ==)
// Assert that x != y.
#define ASSERT_NEQ(x, y) ASSERT_OP(x, y, !=)
// Assert that x < y.
#define ASSERT_LT(x, y)  ASSERT_OP(x, y, <)
// Assert that x <= y.
#define ASSERT_LEQ(x, y) ASSERT_OP(x, y, <=)
// Assert that x > y.
#define ASSERT_GT(x, y)  ASSERT_OP(x, y, >)
// Assert that x >= y.
#define ASSERT_GEQ(x, y) ASSERT_OP(x, y, >=)

// Expect any expression.
#define EXPECT(exp) \
do { \
    if (!(exp)) { \
        test.register_assertion(::unitt::assertion_type::failure, "[FAIL] Expectation not met, expected that (" TOSTRING(exp) ") == true, which is not the case\n"); \
    } else { \
        test.register_assertion(::unitt::assertion_type::success, "[SUCCESS] Expectation met, expected (" TOSTRING(exp) ") == true, which is the case\n"); \
    } \
} while (false)

// Expectation involving operator.
#define EXPECT_OP(x, y, op) \
do { \
    if (!((x) op (y))) { \
        test.register_assertion(::unitt::assertion_type::failure, "[FAIL] Expectation not met, expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is not the case " \
            "[" TOSTRING(x) " = {}, " TOSTRING(y) " = {}]\n", x, y); \
    } else { \
        test.register_assertion(::unitt::assertion_type::success, "[SUCCESS] Expectation met, expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is the case\n"); \
    } \
} while (false)

// Expect that x == y.
#define EXPECT_EQ(x, y)  EXPECT_OP(x, y, ==)
// Expect that x != y.
#define EXPECT_NEQ(x, y) EXPECT_OP(x, y, !=)
// Expect that x < y.
#define EXPECT_LT(x, y)  EXPECT_OP(x, y, <)
// Expect that x <= y.
#define EXPECT_LEQ(x, y) EXPECT_OP(x, y, <=)
// Expect that x > y.
#define EXPECT_GT(x, y)  EXPECT_OP(x, y, >)
// Expect that x >= y.
#define EXPECT_GEQ(x, y) EXPECT_OP(x, y, >=)
