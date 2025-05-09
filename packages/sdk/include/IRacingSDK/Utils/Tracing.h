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

// clang-format off
#include <windows.h>
// clang-format on

#include <exception>
#include <source_location>

#include <TraceLoggingActivity.h>
#include <TraceLoggingProvider.h>

#include "SDKMacros.h"

namespace IRacingSDK {

TRACELOGGING_DECLARE_PROVIDER(gTraceProvider);

#define IRSDK_CPP_TraceLoggingSourceLocation(loc) \
  TraceLoggingValue((loc).file_name(), "File"), \
    TraceLoggingValue((loc).line(), "Line"), \
    TraceLoggingValue((loc).function_name(), "Function")

// TraceLoggingWriteStart() requires the legacy preprocessor :(
static_assert(_MSVC_TRADITIONAL);
// Rewrite these macros if this fails, as presumably the above was fixed :)
//
// - ##__VA_ARGS__             (common vendor extension)
// + __VA_OPT__(,) __VA_ARGS__ (standard C++20)
static_assert(!IRSDK_CPP_VA_OPT_SUPPORTED);
// ... but we currently depend on ##__VA_ARGS__
static_assert(IRSDK_CPP_HAVE_NONSTANDARD_VA_ARGS_COMMA_ELISION);

/** Create and automatically start and stop a named activity.
 *
 * @param OKBTL_ACTIVITY the local variable to store the activity in
 * @param IRSDK_CPP_NAME the name of the activity (C string literal)
 *
 * @see IRSDK_CPP_TraceLoggingScope if you don't need the local variable
 *
 * This avoids templates and `auto` and generally jumps through hoops so that it
 * is valid both inside an implementation, and in a class definition.
 */
#define IRSDK_CPP_TraceLoggingScopedActivity( \
  OKBTL_ACTIVITY, IRSDK_CPP_NAME, ...) \
  const std::function<void(TraceLoggingThreadActivity<gTraceProvider>&)> \
    IRSDK_CPP_CONCAT2(_StartImpl, OKBTL_ACTIVITY) \
    = [&, loc = std::source_location::current()]( \
        TraceLoggingThreadActivity<gTraceProvider>& activity) { \
        TraceLoggingWriteStart( \
          activity, \
          IRSDK_CPP_NAME, \
          IRSDK_CPP_TraceLoggingSourceLocation(loc), \
          ##__VA_ARGS__); \
      }; \
  class IRSDK_CPP_CONCAT2(_Impl, OKBTL_ACTIVITY) final \
    : public TraceLoggingThreadActivity<gTraceProvider> { \
   public: \
    IRSDK_CPP_CONCAT2(_Impl, OKBTL_ACTIVITY) \
    (decltype(IRSDK_CPP_CONCAT2(_StartImpl, OKBTL_ACTIVITY))& startImpl) { \
      startImpl(*this); \
    } \
    IRSDK_CPP_CONCAT2(~_Impl, OKBTL_ACTIVITY)() { \
      if (mAutoStop) { \
        this->Stop(); \
      } \
    } \
    void Stop() { \
      if (mStopped) [[unlikely]] { \
        OutputDebugStringW(L"Double-stopped in Stop()"); \
        IRSDK_CPP_BREAK; \
        return; \
      } \
      mStopped = true; \
      mAutoStop = false; \
      const auto exceptionCount = std::uncaught_exceptions(); \
      if (exceptionCount) [[unlikely]] { \
        TraceLoggingWriteStop( \
          *this, \
          IRSDK_CPP_NAME, \
          TraceLoggingValue(exceptionCount, "UncaughtExceptions")); \
      } else { \
        TraceLoggingWriteStop(*this, IRSDK_CPP_NAME); \
      } \
    } \
    void CancelAutoStop() { \
      mAutoStop = false; \
    } \
    _IRSDK_CPP_TRACELOGGING_IMPL_StopWithResult(IRSDK_CPP_NAME, int); \
    _IRSDK_CPP_TRACELOGGING_IMPL_StopWithResult(IRSDK_CPP_NAME, const char*); \
\
   private: \
    bool mStopped {false}; \
    bool mAutoStop {true}; \
  }; \
  IRSDK_CPP_CONCAT2(_Impl, OKBTL_ACTIVITY) \
  OKBTL_ACTIVITY {IRSDK_CPP_CONCAT2(_StartImpl, OKBTL_ACTIVITY)};

// Not using templates as they're not permitted in local classes
#define _IRSDK_CPP_TRACELOGGING_IMPL_StopWithResult( \
  IRSDK_CPP_NAME, OKBTL_RESULT_TYPE) \
  void StopWithResult(OKBTL_RESULT_TYPE result) { \
    if (mStopped) [[unlikely]] { \
      OutputDebugStringW(L"Double-stopped in StopWithResult()"); \
      IRSDK_CPP_BREAK; \
      return; \
    } \
    this->CancelAutoStop(); \
    mStopped = true; \
    TraceLoggingWriteStop( \
      *this, IRSDK_CPP_NAME, TraceLoggingValue(result, "Result")); \
  }

/** Create and automatically start and stop a named activity.
 *
 * Convenience wrapper around IRSDK_CPP_TraceLoggingScopedActivity
 * that generates the local variable names.
 *
 * @param IRSDK_CPP_NAME the name of the activity (C string literal)
 */
#define IRSDK_CPP_TraceLoggingScope(IRSDK_CPP_NAME, ...) \
  IRSDK_CPP_TraceLoggingScopedActivity( \
    IRSDK_CPP_CONCAT2(_okbtlsa, __COUNTER__), IRSDK_CPP_NAME, ##__VA_ARGS__)

#define IRSDK_CPP_TraceLoggingWrite(IRSDK_CPP_NAME, ...) \
  TraceLoggingWrite( \
    gTraceProvider, \
    IRSDK_CPP_NAME, \
    TraceLoggingValue(__FILE__, "File"), \
    TraceLoggingValue(__LINE__, "Line"), \
    TraceLoggingValue(__FUNCTION__, "Function"), \
    ##__VA_ARGS__)

}// namespace OpenKneeboard
