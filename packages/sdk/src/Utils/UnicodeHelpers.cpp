#include <IRacingSDK/Utils/UnicodeHelpers.h>
#include <unicode/unistr.h>
#include <unicode/ucnv.h>

namespace IRacingSDK::Utils {

  std::string ConvertToUTF8(const std::string& input, const std::string& fromEncoding) {
    UErrorCode status = U_ZERO_ERROR;
    UConverter* conv = ucnv_open(fromEncoding.c_str(), &status);
    if (U_FAILURE(status)) throw std::runtime_error("ucnv_open failed");

    auto ustr = icu::UnicodeString::fromUTF8(input);
    std::string utf8;
    ustr.toUTF8String(utf8);

    ucnv_close(conv);
    return utf8;
  }

  std::string ConvertISO88591ToUTF8(const std::string& input) {
    UErrorCode status = U_ZERO_ERROR;

    // Create converter for ISO-8859-1 (Latin-1)
    UConverter* srcConv = ucnv_open("ISO-8859-1", &status);
    if (U_FAILURE(status)) throw std::runtime_error("Failed to open ISO-8859-1 converter");

    // Convert ISO-8859-1 bytes to UnicodeString (UTF-16 internally)
    int32_t utf16_len = ucnv_toUChars(srcConv, nullptr, 0, input.data(), input.size(), &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
      ucnv_close(srcConv);
      throw std::runtime_error("Failed to calculate buffer size for UTF-16");
    }

    status = U_ZERO_ERROR;
    std::u16string utf16(utf16_len, 0);
    ucnv_toUChars(srcConv, reinterpret_cast<UChar*>(&utf16[0]), utf16_len, input.data(), input.size(), &status);
    ucnv_close(srcConv);
    if (U_FAILURE(status)) throw std::runtime_error("Conversion to UTF-16 failed");

    // Convert UTF-16 to UTF-8
    icu::UnicodeString unicode_str(reinterpret_cast<const UChar*>(utf16.data()), utf16.size());
    std::string utf8;
    unicode_str.toUTF8String(utf8);
    return utf8;
  }
}