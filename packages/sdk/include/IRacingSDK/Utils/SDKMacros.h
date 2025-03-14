/*
 * OpenKneeboard
 *
 * Copyright (C) 2022 Fred Emmott <fred@fredemmott.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */
#pragma once

#include <cassert>

#define IRSDK_CPP_CONCAT1(x, y) x##y
#define IRSDK_CPP_CONCAT2(x, y) IRSDK_CPP_CONCAT1(x, y)

#define IRSDK_CPP_STRINGIFY1(x) #x
#define IRSDK_CPP_STRINGIFY2(x) IRSDK_CPP_STRINGIFY1(x)

// Helper for testing __VA_ARG__ behavior
#define IRSDK_CPP_THIRD_ARG(a, b, c, ...) c

#define IRSDK_CPP_VA_OPT_SUPPORTED_IMPL(...) \
  IRSDK_CPP_THIRD_ARG(__VA_OPT__(, ), true, false, __VA_ARGS__)
#define IRSDK_CPP_VA_OPT_SUPPORTED IRSDK_CPP_VA_OPT_SUPPORTED_IMPL(JUNK)

#define IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION_HELPER(X, ...) \
  X##__VA_ARGS__
#define IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION \
  IRSDK_CPP_THIRD_ARG( \
    IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION_HELPER(JUNK), \
    false, \
    true)

#if IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION
static_assert(
  IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION_HELPER(123) == 123);
#endif

// #pragma section(".CRT$XCU",read)
// #define INITIALIZER(f) \
//     static void f(void); \
//     __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
//     __pragma(comment(linker,"/include:" #f "_")) \
//     static void f(void)

#if defined(__cplusplus)
#define ASSERT_MSG(exp, msg) assert(((void)msg, exp))


#define INITIALIZER(fn)                                                     \
  static void __cdecl fn();                                                 \
  struct napi_init_helper {                                                 \
    napi_init_helper() { fn(); }                                            \
  };                                                                        \
  static napi_init_helper init_helper;                                      \
  static void __cdecl fn()
#else
#pragma section(".CRT$XCU", read)
#define INITIALIZER(fn)                                                     \
  static void __cdecl fn(void);                                             \
  __declspec(dllexport, allocate(".CRT$XCU")) void(__cdecl * fn##_)(void) = \
      fn;                                                                   \
  static void __cdecl fn(void)
#endif    


// #ifdef _WIN64
//     #define (f) INITIALIZER2_(f,"")
// #else
//     #define INITIALIZER(f) INITIALIZER2_(f,"_")
// #endif
// #ifdef __cplusplus
//     #define INITIALIZER(f) \
//         static void f(void); \
//         struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
//         static void f(void)
// #elif defined(_MSC_VER)
//     #pragma section(".CRT$XCU",read)
//     #define INITIALIZER2_(f,p) \
//         static void f(void); \
//         __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
//         __pragma(comment(linker,"/include:" p #f "_")) \
//         static void f(void)
//     #ifdef _WIN64
//         #define INITIALIZER(f) INITIALIZER2_(f,"")
//     #else
//         #define INITIALIZER(f) INITIALIZER2_(f,"_")
//     #endif
// #else
//     #define INITIALIZER(f) \
//         static void f(void) __attribute__((constructor)); \
//         static void f(void)
// #endif

