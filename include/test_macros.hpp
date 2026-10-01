#define STRINGIFY__(x) #x
#define EXPAND__(x) x
#define GLUE_IMPL__(_0, _1) _0##_1
#define GLUE__(_0, _1) GLUE_IMPL__(_0, _1)

#if _MSVC_TRADITIONAL // TODO: The below replacement for __VA_OPT__ doesn't work, fix this (maybe).

    #define VA_CALL__(macro, suffix, ...) EXPAND__(GLUE__(macro, suffix)(__VA_ARGS__))
    #define VA_SELECT__(macro, ...) EXPAND__(VA_CALL__(macro, HAS_ARGS__(__VA_ARGS__), __VA_ARGS__))

    #define HAS_ARGS_IMPL__(_0, _1, _2, _3, _4, _5, N) N
    #define HAS_ARGS__(...) EXPAND__(HAS_ARGS_IMPL__(0, ##__VA_ARGS__, ARGS, ARGS, ARGS, ARGS, ARGS, NOARGS))

    #define COUNT_ARGS__(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N) N
    #define VA_COUNT__(...) EXPAND__(COUNT_ARGS__(0, __VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

    #define APPLY_FEATURES__NOARGS() test_features::none
    #define APPLY_FEATURES__ARGS(...) __VA_ARGS__

    // Syntactic sugar for declaring test inside test group.
    #define TEST(test_name, ...) context.register_test(test_name, EXPAND__(VA_SELECT__(APPLY_FEATURES__, __VA_ARGS__)), [](::unitt::threading_context& test) {

#else
    
    // Specify fixture creation routine for given symbol (group or test), treat it as function definition (continue with curly braces after macro).
    // When provided name is a group name (any typename from Group namespace), it specifies the value that will be used for that group's fixture ("group" parameter inside unit test).
    //   Group fixture is created right before first test of that group is encountered and ran, and destroyed right after the last test of that group has finished execution.
    //   It's passed as const reference to tests, because it should not be modified inside them, but if you really want, it is valid to perform a const_cast on it, 
    //   it's not recommended though as having shared mutable state is against unit testing nature.
    // When provided name is a test typename, it specifies the value that will be used for that test's fixture ("fixture" parameter inside unit test).
    //   Test fixture is created right before running the test and destroyed right after the test has finished execution.
    #define FIXTURE(symbol) \
        namespace unitt { \
            template <> \
            struct fixture_creator<symbol> { \
                [[nodiscard]] static decltype(auto) create(); \
            }; \
        } \
        decltype(auto) ::unitt::fixture_creator<symbol>::create()

#endif

#define GLOBAL_TEST_IMPL__(original_name, original_group, generated_name) \
    namespace original_group { \
        struct original_name; \
    } \
    namespace Group { \
        struct original_group; \
    } \
    struct generated_name { \
        constexpr static inline std::string_view name = #original_name; \
        using test_type = ::original_group::original_name; \
        using group_type = ::Group::original_group; \
        static void run(::unitt::threading_context&, auto& fixture, const auto& group); \
    }; \
    const static ::unitt::test_registrar<generated_name> GLUE__(generated_name, registrar__){}; \
    void generated_name::run(::unitt::threading_context& test, auto& fixture, const auto& group)

// Define a test which belongs to given group, this test can be referenced in other macros using a combination of group name and test name (passed as separate arguments). 
#define TEST(non_string_group, non_string_name) GLOBAL_TEST_IMPL__(non_string_name, non_string_group, non_string_group##_##non_string_name##__)

#if _MSVC_TRADITIONAL
    #define TEST_MESSAGE(message, ...) test.fmessage(std::string_view{ message }, __VA_ARGS__)
#else
    #define TEST_MESSAGE(message, ...) test.fmessage(std::string_view{ message } __VA_OPT__(,) __VA_ARGS__)
#endif
#define TEST_VMESSAGE(...) test.variadic_message(__VA_ARGS__)

#define ENCODERS(...) \
    namespace unitt { \
        template <> \
        struct encoders<global_encoders> { \
            using type = type_list<__VA_ARGS__>; \
        }; \
    }

// Expect any expression.
#define EXPECT(exp) \
do { \
    if (!(exp)) { \
        test.register_assertion(::unitt::assertion_result::failure, "[FAIL] Expectation not met, expected that (" TOSTRING(exp) ") == true, which is not the case\n"); \
    } else { \
        test.register_assertion(::unitt::assertion_result::success, "[SUCCESS] Expectation met, expected (" TOSTRING(exp) ") == true, which is the case\n"); \
    } \
} while (false)

// Expectation involving operator.
#define EXPECT_OP(x, y, op) \
do { \
    if (!((x) op (y))) { \
        test.register_assertion(::unitt::assertion_result::failure, "[FAIL] Expectation not met, expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is not the case " \
            "[" TOSTRING(x) " = {}, " TOSTRING(y) " = {}]\n", x, y); \
    } else { \
        test.register_assertion(::unitt::assertion_result::success, "[SUCCESS] Expectation met, expected that (" TOSTRING(x) ") " TOSTRING(op) " (" TOSTRING(y) "), which is the case\n"); \
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