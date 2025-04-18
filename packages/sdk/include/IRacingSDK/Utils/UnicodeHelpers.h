

// Original copyright
////////////////////////////////////////////////////////////////////////////////
// Unicode UTF-16/UTF-8 conversion functions for std::wstring and std::string
//
//                  Copyright (C) by Giovanni Dicanio
//                    <giovanni.dicanio AT gmail.com>
//
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// This is a header-only C++ file that implements a couple of functions
// to simply and conveniently convert Unicode text between UTF-16 and UTF-8.
//
// std::wstring is used to store UTF-16-encoded text.
// std::string is used to store UTF-8-encoded text.
//
// The exported functions are:
//
//      * Convert from UTF-16 to UTF-8:
//        std::string ToUtf8(std::wstring const& utf16)
//
//      * Convert from UTF-8 to UTF-16:
//        std::wstring ToUtf16(std::string const& utf8)
//
// These functions live under the UnicodeConvStd namespace.
//
// This code compiles cleanly at warning level 4 (/W4)
// on both 32-bit and 64-bit builds with Visual Studio 2019 in C++17 mode.
//
//
//------------------------------------------------------------------------------
//
// The MIT License(MIT)
//
// Copyright(c) 2016-2023 by Giovanni Dicanio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//------------------------------------------------------------------------------


//==============================================================================
//                              Includes
//==============================================================================

#pragma once
#include <windows.h>    // Win32 Platform SDK

#include <crtdbg.h>     // _ASSERTE

#include <limits>       // std::numeric_limits
#include <stdexcept>    // std::runtime_error, std::overflow_error
#include <string>       // std::string, std::wstring


//==============================================================================
//                              Implementation
//==============================================================================

namespace IRacingSDK::Utils {

//------------------------------------------------------------------------------
// Represents an error during Unicode conversions
//------------------------------------------------------------------------------
class UnicodeConversionException
    : public std::runtime_error
{
public:

    enum class ConversionType
    {
        FromUtf16ToUtf8,
        FromUtf8ToUtf16
    };

    UnicodeConversionException(DWORD errorCode, ConversionType conversionType, const char* message)
        : std::runtime_error(message),
        m_errorCode(errorCode),
        m_conversionType(conversionType)
    {
    }

    UnicodeConversionException(DWORD errorCode, ConversionType conversionType, const std::string& message)
        : std::runtime_error(message),
        m_errorCode(errorCode),
        m_conversionType(conversionType)
    {
    }

     DWORD GetErrorCode() const noexcept
    {
        return m_errorCode;
    }

     ConversionType GetConversionType() const noexcept
    {
        return m_conversionType;
    }

private:
    DWORD m_errorCode;
    ConversionType m_conversionType;
};


namespace Details
{

//------------------------------------------------------------------------------
// Helper function to safely convert a size_t value to int.
// If size_t is too large, throws a std::overflow_error.
//------------------------------------------------------------------------------
inline int SafeToInt(size_t s)
{
    using DestinationType = int;

    if (s > static_cast<size_t>((std::numeric_limits<DestinationType>::max)()))
    {
        throw std::overflow_error(
            "Input size is too long: size_t-length doesn't fit into int.");
    }

    return static_cast<DestinationType>(s);
}

} // namespace Details


//------------------------------------------------------------------------------
// Convert from UTF-16 std::wstring to UTF-8 std::string.
// Signal errors throwing UnicodeConversionException.
//------------------------------------------------------------------------------
inline  std::string ToUtf8(std::wstring const& utf16)
{
    // Special case of empty input string
    if (utf16.empty())
    {
        // Empty input --> return empty output string
        return std::string{};
    }

    // Safely fail if an invalid UTF-16 character sequence is encountered
    constexpr DWORD kFlags = WC_ERR_INVALID_CHARS;

    const int utf16Length = Details::SafeToInt(utf16.length());

    // Get the length, in chars, of the resulting UTF-8 string
    const int utf8Length = ::WideCharToMultiByte(
        CP_UTF8,            // convert to UTF-8
        kFlags,             // conversion flags
        utf16.data(),       // source UTF-16 string
        utf16Length,        // length of source UTF-16 string, in wchar_ts
        nullptr,            // unused - no conversion required in this step
        0,                  // request size of destination buffer, in chars
        nullptr, nullptr    // unused
    );
    if (utf8Length == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD errorCode = ::GetLastError();
        throw UnicodeConversionException(
            errorCode,
            UnicodeConversionException::ConversionType::FromUtf16ToUtf8,
            "Can't get result UTF-8 string length (WideCharToMultiByte failed).");
    }

    // Make room in the destination string for the converted bits
    std::string utf8(utf8Length, ' ');
    char* utf8Buffer = utf8.data();
    _ASSERTE(utf8Buffer != nullptr);

    // Do the actual conversion from UTF-16 to UTF-8
    int result = ::WideCharToMultiByte(
        CP_UTF8,            // convert to UTF-8
        kFlags,             // conversion flags
        utf16.data(),       // source UTF-16 string
        utf16Length,        // length of source UTF-16 string, in wchar_ts
        utf8Buffer,         // pointer to destination buffer
        utf8Length,         // size of destination buffer, in chars
        nullptr, nullptr    // unused
    );
    if (result == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD errorCode = ::GetLastError();
        throw UnicodeConversionException(
            errorCode,
            UnicodeConversionException::ConversionType::FromUtf16ToUtf8,
            "Can't convert from UTF-16 to UTF-8 string (WideCharToMultiByte failed).");
    }

    return utf8;
}


//------------------------------------------------------------------------------
// Convert from UTF-8 std::string to UTF-16 std::wstring.
// Signal errors throwing UnicodeConversionException.
//------------------------------------------------------------------------------
inline  std::wstring ToUtf16(std::string const& utf8)
{
    // Special case of empty input string
    if (utf8.empty())
    {
        // Empty input --> return empty output string
        return std::wstring{};
    }

    // Safely fail if an invalid UTF-8 character sequence is encountered
    constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;

    const int utf8Length = Details::SafeToInt(utf8.length());

    // Get the size of the destination UTF-16 string
    const int utf16Length = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8.data(),   // source UTF-8 string pointer
        utf8Length,    // length of the source UTF-8 string, in chars
        nullptr,       // unused - no conversion done in this step
        0              // request size of destination buffer, in wchar_ts
    );
    if (utf16Length == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD errorCode = ::GetLastError();
        throw UnicodeConversionException(
            errorCode,
            UnicodeConversionException::ConversionType::FromUtf8ToUtf16,
            "Can't get result UTF-16 string length (MultiByteToWideChar failed).");
    }

    // Make room in the destination string for the converted bits
    std::wstring utf16(utf16Length, L' ');
    wchar_t* utf16Buffer = utf16.data();
    _ASSERTE(utf16Buffer != nullptr);

    // Do the actual conversion from UTF-8 to UTF-16
    int result = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8.data(),   // source UTF-8 string pointer
        utf8Length,    // length of source UTF-8 string, in chars
        utf16Buffer,   // pointer to destination buffer
        utf16Length    // size of destination buffer, in wchar_ts
    );
    if (result == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD errorCode = ::GetLastError();
        throw UnicodeConversionException(
            errorCode,
            UnicodeConversionException::ConversionType::FromUtf8ToUtf16,
            "Can't convert from UTF-8 to UTF-16 string (MultiByteToWideChar failed).");
    }

    return utf16;
}

} // namespace UnicodeConvStd


