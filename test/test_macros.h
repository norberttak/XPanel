/*
 * Copyright 2026 Norbert Takacs
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

/*
 * Minimal shim that maps the small subset of the MS CppUnitTest macros used by
 * test_*.cpp (TEST_CLASS, TEST_METHOD, TEST_METHOD_INITIALIZE, TEST_METHOD_CLEANUP
 * and Assert::AreEqual/AreNotEqual/IsTrue/IsFalse) onto doctest, so the same test
 * sources can be built with CMake/ctest on macOS/Linux in addition to the
 * Visual Studio test project on Windows.
 */

#include "doctest.h"
#include <cstring>
#include <type_traits>

namespace test_shim {

// Default no-op setup/teardown hooks; TEST_METHOD_INITIALIZE/TEST_METHOD_CLEANUP
// override these (under their fixed names) when present in a TEST_CLASS.
struct FixtureBase
{
    void __test_setup() {}
    void __test_teardown() {}
};

// Instantiates the fixture, runs setup/method/teardown. Used as the doctest
// test case function (a plain function pointer, so the method to call has to
// be a template parameter rather than a captured value).
template <typename T, void (T::*Method)()>
void run_test_method()
{
    T instance{};
    instance.__test_setup();
    (instance.*Method)();
    instance.__test_teardown();
}

} // namespace test_shim

// TEST_CLASS(name) is followed by a literal "{ ... };" written by the test
// author, so this macro must not open a brace itself. The two declarations
// below give TEST_METHOD access to the enclosing class type and its name
// without needing extra arguments.
#define TEST_CLASS(name)                                  \
    struct name;                                          \
    using _CurrentTestClass_ = name;                      \
    inline constexpr const char* _CurrentTestSuite_ = #name; \
    struct name : public test_shim::FixtureBase

// CppUnitTest invokes these by their own (arbitrary) names before/after every
// TEST_METHOD; here they are simply mapped onto fixed method names.
#define TEST_METHOD_INITIALIZE(name) void __test_setup()
#define TEST_METHOD_CLEANUP(name) void __test_teardown()

// Declares the test method and registers it with doctest. The registration
// struct's constructor body is a "complete-class context", so it can refer to
// _CurrentTestClass_::name even though "name" itself is declared further down.
#define TEST_METHOD(name)                                                            \
    struct name##_registrar                                                         \
    {                                                                                \
        name##_registrar()                                                          \
        {                                                                           \
            doctest::detail::regTest(                                              \
                doctest::detail::TestCase(                                          \
                    &test_shim::run_test_method<_CurrentTestClass_, &_CurrentTestClass_::name>, \
                    __FILE__, __LINE__,                                             \
                    doctest::detail::TestSuite() * _CurrentTestSuite_)              \
                * #name);                                                           \
        }                                                                           \
    };                                                                              \
    inline static name##_registrar name##_registrar_instance{};                    \
    void name()

// Allows "using namespace Microsoft::VisualStudio::CppUnitTestFramework;" in
// the test sources to compile unchanged; Assert lives in the global namespace
// below, so nothing further needs to be pulled in by that using-directive.
namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework {} } }

// CppUnitTest's Assert::* throw on failure, aborting the current TEST_METHOD
// (but not the whole run); doctest's REQUIRE_* macros do the same.
namespace Assert {

namespace detail {

// Detects (decayed) char*/const char* arguments, including string literals
// and char arrays, which CppUnitTest's AreEqual compares by content rather
// than by pointer/array identity.
template <typename T>
inline constexpr bool is_c_string_v =
    std::is_same_v<std::decay_t<T>, char*> || std::is_same_v<std::decay_t<T>, const char*>;

} // namespace detail

template <typename TExpected, typename TActual>
void AreEqual(const TExpected& expected, const TActual& actual)
{
    if constexpr (detail::is_c_string_v<TExpected> && detail::is_c_string_v<TActual>)
        REQUIRE(std::strcmp(expected, actual) == 0);
    else
        REQUIRE_EQ(expected, actual);
}

// CppUnitTest's AreEqual(expected, actual, message, lineInfo) overload; the
// message/line info are dropped, doctest reports its own location.
template <typename TExpected, typename TActual>
void AreEqual(const TExpected& expected, const TActual& actual, const wchar_t* message, long line_info = 0)
{
    (void)message;
    (void)line_info;
    AreEqual(expected, actual);
}

template <typename TExpected, typename TActual>
void AreNotEqual(const TExpected& expected, const TActual& actual)
{
    if constexpr (detail::is_c_string_v<TExpected> && detail::is_c_string_v<TActual>)
        REQUIRE(std::strcmp(expected, actual) != 0);
    else
        REQUIRE_NE(expected, actual);
}

inline void IsTrue(bool condition)
{
    REQUIRE(condition);
}

inline void IsFalse(bool condition)
{
    REQUIRE_FALSE(condition);
}

} // namespace Assert
