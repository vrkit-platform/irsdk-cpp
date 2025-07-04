//
// Created by jglanz on 1/5/2024.
//

#pragma once

#include <IRacingSDK/Utils/Win32.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <source_location>


#ifdef DEBUG
#define IRSDKCPP_BREAK
  __debugbreak()
#else
  #define IRSDKCPP_BREAK
#endif

#define IRSDKCPP_TUPLE_PROP(name, index) \
  auto& name() {\
    return std::get<index>(*this);\
  }

namespace IRacingSDK::Examples {

  inline void fatal [[noreturn]]() {
    // The FAST_FAIL_FATAL_APP_EXIT macro is defined in winnt.h, but we don't want
    // to pull that in here...
    constexpr unsigned int fast_fail_fatal_app_exit = 7;
    __fastfail(fast_fail_fatal_app_exit);
  }

#define IRSDKCPP_FATAL \
  { fatal(); }


#define IRSDKCPP_LOG_SOURCE_LOCATION_AND_THROW(\
  message, ...) \
{ \
auto msg = std::format("ERROR: " message "\n", ##__VA_ARGS__); \
std::cerr << msg << "\n";  \
throw std::runtime_error(msg); \
}


#define IRSDKCPP_LOG_SOURCE_LOCATION_AND_FATAL(\
  message, ...) \
  { \
    auto msg = std::format("FATAL: " message "\n", ##__VA_ARGS__); \
    std::cerr << msg << "\n";  \
    IRSDKCPP_FATAL; \
  }

  /** Use this if something is so wrong that we're almost certainly going to
   * crash.
   *
   * Crashing earlier is better than crashing later, as we get more usable
   * debugging information.
   *
   * @see `IRSDKCPP_LOG_SOURCE_LOCATION_AND_FATAL` if you need to provide a
   * source location.
   */
#define IRSDKCPP_LOG_AND_FATAL(message, ...) \
  IRSDKCPP_LOG_SOURCE_LOCATION_AND_FATAL( \
    message, ##__VA_ARGS__)

#define IRSDKCPP_LOG_AND_FATAL_IF(cond, message, ...) \
  { \
    if (cond) \
      IRSDKCPP_LOG_AND_FATAL(message, ##__VA_ARGS__); \
  }

#define IRSDKCPP_LOG_AND_THROW(message, ...) \
IRSDKCPP_LOG_SOURCE_LOCATION_AND_THROW( \
message, ##__VA_ARGS__)


#define IRSDKCPP_LOG_AND_THROW_IF(cond, message, ...) \
{ \
  if (cond) {\
    IRSDKCPP_LOG_AND_THROW(message, ##__VA_ARGS__); \
  }\
}

#define IRSDKCPP_LOG_AND_THROW_IF_FAILED(cond, message, ...) \
{ \
if (FAILED(cond)) {\
IRSDKCPP_LOG_AND_THROW(message, ##__VA_ARGS__); \
}\
}


  inline void check_hresult(HRESULT code) {
    //winrt::check_hresult(code);
    if (FAILED(code)) [[unlikely]] {
      // IRSDKCPP_LOG_SOURCE_LOCATION_AND_FATAL("HRESULT {}", static_cast<int32_t>(code));
      IRSDKCPP_LOG_SOURCE_LOCATION_AND_FATAL("HRESULT {}", static_cast<int32_t>(code));
    }
  }

  namespace Win32 = IRacingSDK::Utils::Win32;

} // namespace VRKit::Shared

/******************************************************************
 *                                                                 *
 *  Macros                                                         *
 *                                                                 *
 ******************************************************************/

// #define LOGD(s) \
// { \
// auto c = s.c_str();\
// OutputDebugStringA(c); \
// }

#ifndef Assert
#if defined(DEBUG) ||defined(_DEBUG)
#define Assert(b) \
    if (!(b)) { \
        IRSDKCPP_LOG_AND_FATAL(#b);\
    }
#else
#define Assert(b)
#endif // DEBUG || _DEBUG
#endif

#ifndef AssertMsg
#if defined(DEBUG) ||defined(_DEBUG)
#define AssertMsg(b, msg) \
    if (!(b)) { \
            IRSDKCPP_LOG_AND_FATAL(#b ": {}", msg);\
    }
#else
#define AssertMsg(b, msg)
#endif // DEBUG || _DEBUG
#endif


#define AssertOkMsg(result, msg) AssertMsg(SUCCEEDED(result), msg)
#define AssertOk(result) Assert(SUCCEEDED(result))
#define AOKMSG AssertOkMsg
#define AOK AssertOk


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE) & __ImageBase)
#endif

#define LOCK(Mutex, Var) std::scoped_lock Var(Mutex)
