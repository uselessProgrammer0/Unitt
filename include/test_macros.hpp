// Syntactic sugar for declaring test group.
#define TEST_GROUP(group_name, group_fixture) \
class group_name : public ::unitt::unit_test_group { \
public: \
    using fixture_type = group_fixture; \
    constexpr static inline std::string_view name = #group_name; \
    static void collect(auto& group) {

// We need to close group_name::collect, and group_name.
#define END_TEST_GROUP }};

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

    // Syntactic sugar for declaring test inside test group.
    #define TEST(test_name, ...) group.register_test(test_name, ::unitt::test_features::none __VA_OPT__(| __VA_ARGS__), [](::unitt::threading_context& test) {

#endif

#define END_TEST });

#define GLOBAL_TEST_IMPL__(generated_name, fixture_typename) \
    struct generated_name { \
        using fixture_type = fixture_typename; \
        static void run(::unitt::threading_context&, fixture_type&); \
    }; \
    const static ::unitt::test_registrar<generated_name> generated_name##registrar__{}; \
    void generated_name::run(::unitt::threading_context& test, generated_name::fixture_type& fixture)

#define GLOBAL_TEST(non_string_name, fixture_type) GLOBAL_TEST_IMPL__(GLUE__(non_string_name, GLUE__(__LINE__, __COUNTER__))__, fixture_type)

#define TEST_MESSAGE(message, ...) test.formatted_message(std::string_view{ message }, __VA_ARGS__)
#define TEST_VMESSAGE(...) test.variadic_message(__VA_ARGS__)