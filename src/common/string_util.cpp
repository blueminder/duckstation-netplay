#include "string_util.h"
#include <cctype>
#include <codecvt>
#include <cstdio>
#include <sstream>

#ifdef _WIN32
#include "windows_headers.h"
#endif

std::string StringUtil::StdStringFromFormat(const char* format, ...)
{
  std::va_list ap;
  va_start(ap, format);
  std::string ret = StdStringFromFormatV(format, ap);
  va_end(ap);
  return ret;
}

std::string StringUtil::StdStringFromFormatV(const char* format, std::va_list ap)
{
  std::va_list ap_copy;
  va_copy(ap_copy, ap);

#ifdef _WIN32
  int len = _vscprintf(format, ap_copy);
#else
  int len = std::vsnprintf(nullptr, 0, format, ap_copy);
#endif
  va_end(ap_copy);

  std::string ret;

  // If an encoding error occurs, len is -1. Which we definitely don't want to resize to.
  if (len > 0)
  {
    ret.resize(len);
    std::vsnprintf(ret.data(), ret.size() + 1, format, ap);
  }

  return ret;
}

bool StringUtil::WildcardMatch(const char* subject, const char* mask, bool case_sensitive /*= true*/)
{
  if (case_sensitive)
  {
    const char* cp = nullptr;
    const char* mp = nullptr;

    while ((*subject) && (*mask != '*'))
    {
      if ((*mask != '?') && (std::tolower(*mask) != std::tolower(*subject)))
        return false;

      mask++;
      subject++;
    }

    while (*subject)
    {
      if (*mask == '*')
      {
        if (*++mask == 0)
          return true;

        mp = mask;
        cp = subject + 1;
      }
      else
      {
        if ((*mask == '?') || (std::tolower(*mask) == std::tolower(*subject)))
        {
          mask++;
          subject++;
        }
        else
        {
          mask = mp;
          subject = cp++;
        }
      }
    }

    while (*mask == '*')
    {
      mask++;
    }

    return *mask == 0;
  }
  else
  {
    const char* cp = nullptr;
    const char* mp = nullptr;

    while ((*subject) && (*mask != '*'))
    {
      if ((*mask != *subject) && (*mask != '?'))
        return false;

      mask++;
      subject++;
    }

    while (*subject)
    {
      if (*mask == '*')
      {
        if (*++mask == 0)
          return true;

        mp = mask;
        cp = subject + 1;
      }
      else
      {
        if ((*mask == *subject) || (*mask == '?'))
        {
          mask++;
          subject++;
        }
        else
        {
          mask = mp;
          subject = cp++;
        }
      }
    }

    while (*mask == '*')
    {
      mask++;
    }

    return *mask == 0;
  }
}

std::size_t StringUtil::Strlcpy(char* dst, const char* src, std::size_t size)
{
  std::size_t len = std::strlen(src);
  if (len < size)
  {
    std::memcpy(dst, src, len + 1);
  }
  else
  {
    std::memcpy(dst, src, size - 1);
    dst[size - 1] = '\0';
  }
  return len;
}

std::size_t StringUtil::Strlcpy(char* dst, const std::string_view& src, std::size_t size)
{
  std::size_t len = src.length();
  if (len < size)
  {
    std::memcpy(dst, src.data(), len);
    dst[len] = '\0';
  }
  else
  {
    std::memcpy(dst, src.data(), size - 1);
    dst[size - 1] = '\0';
  }
  return len;
}

std::optional<std::vector<u8>> StringUtil::DecodeHex(const std::string_view& in)
{
  std::vector<u8> data;
  data.reserve(in.size() / 2);

  for (size_t i = 0; i < in.size() / 2; i++)
  {
    std::optional<u8> byte = StringUtil::FromChars<u8>(in.substr(i * 2, 2), 16);
    if (byte.has_value())
      data.push_back(*byte);
    else
      return std::nullopt;
  }

  return {data};
}

std::string StringUtil::EncodeHex(const u8* data, int length)
{
  std::stringstream ss;
  for (int i = 0; i < length; i++)
    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[i]);

  return ss.str();
}

std::string_view StringUtil::StripWhitespace(const std::string_view& str)
{
  std::string_view::size_type start = 0;
  while (start < str.size() && std::isspace(str[start]))
    start++;
  if (start == str.size())
    return {};

  std::string_view::size_type end = str.size() - 1;
  while (end > start && std::isspace(str[end]))
    end--;

  return str.substr(start, end - start + 1);
}

void StringUtil::StripWhitespace(std::string* str)
{
  {
    const char* cstr = str->c_str();
    std::string_view::size_type start = 0;
    while (start < str->size() && std::isspace(cstr[start]))
      start++;
    if (start != 0)
      str->erase(0, start);
  }

  {
    const char* cstr = str->c_str();
    std::string_view::size_type start = str->size();
    while (start > 0 && std::isspace(cstr[start - 1]))
      start--;
    if (start != str->size())
      str->erase(start);
  }
}

std::vector<std::string_view> StringUtil::SplitString(const std::string_view& str, char delimiter,
                                                      bool skip_empty /*= true*/)
{
  std::vector<std::string_view> res;
  std::string_view::size_type last_pos = 0;
  std::string_view::size_type pos;
  while (last_pos < str.size() && (pos = str.find(delimiter, last_pos)) != std::string_view::npos)
  {
    std::string_view part(StripWhitespace(str.substr(last_pos, pos - last_pos)));
    if (!skip_empty || !part.empty())
      res.push_back(std::move(part));

    last_pos = pos + 1;
  }

  if (last_pos < str.size())
  {
    std::string_view part(StripWhitespace(str.substr(last_pos)));
    if (!skip_empty || !part.empty())
      res.push_back(std::move(part));
  }

  return res;
}

std::string StringUtil::ReplaceAll(const std::string_view& subject, const std::string_view& search,
                                   const std::string_view& replacement)
{
  std::string ret(subject);
  ReplaceAll(&ret, search, replacement);
  return ret;
}

void StringUtil::ReplaceAll(std::string* subject, const std::string_view& search, const std::string_view& replacement)
{
  if (!subject->empty())
  {
    std::string::size_type start_pos = 0;
    while ((start_pos = subject->find(search, start_pos)) != std::string::npos)
    {
      subject->replace(start_pos, search.length(), replacement);
      start_pos += replacement.length();
    }
  }
}

bool StringUtil::ParseAssignmentString(const std::string_view& str, std::string_view* key, std::string_view* value)
{
  const std::string_view::size_type pos = str.find('=');
  if (pos == std::string_view::npos)
  {
    *key = std::string_view();
    *value = std::string_view();
    return false;
  }

  *key = StripWhitespace(str.substr(0, pos));
  if (pos != (str.size() - 1))
    *value = StripWhitespace(str.substr(pos + 1));
  else
    *value = std::string_view();

  return true;
}

void StringUtil::EncodeAndAppendUTF8(std::string& s, char32_t ch)
{
  if (ch <= 0x7F)
  {
    s.push_back(static_cast<char>(static_cast<u8>(ch)));
  }
  else if (ch <= 0x07FF)
  {
    s.push_back(static_cast<char>(static_cast<u8>(0xc0 | static_cast<u8>((ch >> 6) & 0x1f))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>((ch & 0x3f)))));
  }
  else if (ch <= 0xFFFF)
  {
    s.push_back(static_cast<char>(static_cast<u8>(0xe0 | static_cast<u8>(((ch >> 12) & 0x0f)))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>(((ch >> 6) & 0x3f)))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>((ch & 0x3f)))));
  }
  else if (ch <= 0x10FFFF)
  {
    s.push_back(static_cast<char>(static_cast<u8>(0xf0 | static_cast<u8>(((ch >> 18) & 0x07)))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>(((ch >> 12) & 0x3f)))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>(((ch >> 6) & 0x3f)))));
    s.push_back(static_cast<char>(static_cast<u8>(0x80 | static_cast<u8>((ch & 0x3f)))));
  }
  else
  {
    s.push_back(static_cast<char>(0xefu));
    s.push_back(static_cast<char>(0xbfu));
    s.push_back(static_cast<char>(0xbdu));
  }
}

size_t StringUtil::DecodeUTF8(const void* bytes, size_t length, char32_t* ch)
{
  const u8* s = reinterpret_cast<const u8*>(bytes);
  if (s[0] < 0x80)
  {
    *ch = s[0];
    return 1;
  }
  else if ((s[0] & 0xe0) == 0xc0)
  {
    if (length < 2)
      goto invalid;

    *ch = static_cast<char32_t>((static_cast<u32>(s[0] & 0x1f) << 6) | (static_cast<u32>(s[1] & 0x3f) << 0));
    return 2;
  }
  else if ((s[0] & 0xf0) == 0xe0)
  {
    if (length < 3)
      goto invalid;

    *ch = static_cast<char32_t>((static_cast<u32>(s[0] & 0x0f) << 12) | (static_cast<u32>(s[1] & 0x3f) << 6) |
                                (static_cast<u32>(s[2] & 0x3f) << 0));
    return 3;
  }
  else if ((s[0] & 0xf8) == 0xf0 && (s[0] <= 0xf4))
  {
    if (length < 4)
      goto invalid;

    *ch = static_cast<char32_t>((static_cast<u32>(s[0] & 0x07) << 18) | (static_cast<u32>(s[1] & 0x3f) << 12) |
                                (static_cast<u32>(s[2] & 0x3f) << 6) | (static_cast<u32>(s[3] & 0x3f) << 0));
    return 4;
  }

invalid:
  *ch = 0xFFFFFFFFu;
  return 1;
}

size_t StringUtil::DecodeUTF8(const std::string_view& str, size_t offset, char32_t* ch)
{
  return DecodeUTF8(str.data() + offset, str.length() - offset, ch);
}

size_t StringUtil::DecodeUTF8(const std::string& str, size_t offset, char32_t* ch)
{
  return DecodeUTF8(str.data() + offset, str.length() - offset, ch);
}

#ifdef _WIN32

std::wstring StringUtil::UTF8StringToWideString(const std::string_view& str)
{
  std::wstring ret;
  if (!UTF8StringToWideString(ret, str))
    return {};

  return ret;
}

bool StringUtil::UTF8StringToWideString(std::wstring& dest, const std::string_view& str)
{
  int wlen = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0);
  if (wlen < 0)
    return false;

  dest.resize(wlen);
  if (wlen > 0 && MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), wlen) < 0)
    return false;

  return true;
}

std::string StringUtil::WideStringToUTF8String(const std::wstring_view& str)
{
  std::string ret;
  if (!WideStringToUTF8String(ret, str))
    return {};

  return ret;
}

bool StringUtil::WideStringToUTF8String(std::string& dest, const std::wstring_view& str)
{
  int mblen = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), nullptr, 0, nullptr, nullptr);
  if (mblen < 0)
    return false;

  dest.resize(mblen);
  if (mblen > 0 && WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.length()), dest.data(), mblen,
                                       nullptr, nullptr) < 0)
  {
    return false;
  }

  return true;
}

#endif
