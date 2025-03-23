#pragma once

// Check if building as a shared library
// #if defined(IRSDKCPP_BUILD_SHARED)
//
// // On Windows
// #  if defined(_WIN32) || defined(_WIN64)
// #    if defined(IRSDKCPP_DLL) // Export when building the DLL
// #      define IRSDKCPP_API __declspec(dllexport)
// #    else                     // Import when consuming the DLL
// #      define IRSDKCPP_API __declspec(dllimport)
// #    endif
// #  else  // On non-Windows platforms (visibility attributes for GCC/Clang)
// #    if __GNUC__ >= 4
// #      define IRSDKCPP_API __attribute__((visibility("default")))
// #    else
// #      define IRSDKCPP_API
// #    endif
// #  endif
//
// // If not building shared, define it as empty, meaning static linking
// #else
// #  define IRSDKCPP_API
// #endif

#  define IRSDKCPP_API
