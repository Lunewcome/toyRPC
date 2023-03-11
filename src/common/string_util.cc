/** 
* @brief    load conf
* @date     2014-03-17
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/
#include "common/string_util.h"

#include "common/dmg_fp.h"

#include <errno.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>

template <class StringType>
static void StringAppendVT(StringType* dst,
                           const typename StringType::value_type* format,
                           va_list ap) {
  const int array_size = 1024;
  typename StringType::value_type stack_buf[array_size];

  va_list ap_copy;
  va_copy(ap_copy, ap);

  errno = 0;
  int result = vsnprintf(stack_buf, array_size, format, ap_copy);
  va_end(ap_copy);

  if (result >= 0 && result < array_size) {
    // It fit.
    dst->append(stack_buf, result);
    return;
  }

  // Repeatedly increase buffer size until it fits.
  int mem_length = array_size;
  while (true) {
    if (result < 0) {
      if (errno != 0 && errno != EOVERFLOW) {
        return;
      }
      // Try doubling the buffer size.
      mem_length *= 2;
    } else {
      // We need exactly "result + 1" characters.
      mem_length = result + 1;
    }

    if (mem_length > 32 * 1024 * 1024) {
      return;
    }

    std::vector<typename StringType::value_type> mem_buf(mem_length);

    va_copy(ap_copy, ap);
    result = vsnprintf(&mem_buf[0], mem_length, format, ap_copy);
    va_end(ap_copy);

    if ((result >= 0) && (result < mem_length)) {
      // It fit.
      dst->append(&mem_buf[0], result);
      return;
    }
  }
}

void StringAppendV(std::string* dst, const char* format, va_list ap) {
    StringAppendVT(dst, format, ap);
}

void StringAppendF(std::string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

void StringPrintf(std::string* dst,
                  const char* format,
                  ...) {
  dst->clear();
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

static int strtoi(const char *nptr, char **endptr, int base) {
  static const int kint32max = ~(1<<31);
  static const int kint32min = 1<<31;
  long long  res = strtol(nptr, endptr, base);
#if __LP64__
  // Long is 64-bits, we have to handle under/overflow ourselves.
  if (res > kint32max) {
    res = kint32max;
    errno = ERANGE;
  } else if (res < kint32min) {
    res = kint32min;
    errno = ERANGE;
  }
#endif
  return static_cast<int>(res);
}

template<typename StringToNumberTraits>
bool StringToNumber(const typename StringToNumberTraits::string_type& input,
                    typename StringToNumberTraits::value_type* output) {
  typedef StringToNumberTraits traits;

  errno = 0;  // Thread-safe?  It is on at least Mac, Linux, and Windows.
  typename traits::string_type::value_type* endptr = NULL;
  typename traits::value_type value = traits::convert_func(input.c_str(),
                                                           &endptr);
  *output = value;
  return errno == 0 &&
         !input.empty() &&
         input.c_str() + input.length() == endptr &&
         traits::valid_func(input);
}

class StringToIntTraits {
 public:
  typedef std::string string_type;
  typedef int value_type;
  static const int kBase = 10;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    return strtoi(str, endptr, kBase); 
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class StringToInt64Traits {
 public:
  typedef std::string string_type;
  typedef int64 value_type;
  static const int kBase = 10;
  static inline value_type convert_func(
      const string_type::value_type* str,
      string_type::value_type** endptr) {
#ifdef OS_WIN
    return _strtoi64(str, endptr, kBase);
#else  // assume OS_POSIX
    return strtoll(str, endptr, kBase);
#endif
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

class StringToDoubleTraits {
 public:
  typedef std::string string_type;
  typedef double value_type;
  static inline value_type convert_func(const string_type::value_type* str,
                                        string_type::value_type** endptr) {
    return dmg_fp::strtod(str, endptr);
  }
  static inline bool valid_func(const string_type& str) {
    return !str.empty() && !isspace(str[0]);
  }
};

bool StringToInt(const std::string& input, int* output) {
  return StringToNumber<StringToIntTraits>(input, output);
}

bool StringToInt64(const std::string& input, int64* output) {
  return StringToNumber<StringToInt64Traits>(input, output);
}

bool StringToDouble(const std::string& input, double* output) {
  return StringToNumber<StringToDoubleTraits>(input, output);
}

void SplitString(const char* in_str,
                 char delemiter,
                 std::vector<std::string>* results) {
    if (!in_str) {
        return;
    }
    size_t len = strlen(in_str);
    if (len == 0) {
        return;
    }
    results->push_back("");
    for (size_t i = 0; i < len; ++i) {
        if (in_str[i] == delemiter) {
            if (i != 0) {
                results->push_back("");
            }
            continue;
        }
        results->back().append(in_str + i, 1);
    }
}

void SplitString(const std::string& in_str,
                 char delemiter,
                 std::vector<std::string>* results) {
    SplitString(in_str.c_str(), delemiter, results);
}

void StringToDouble(const std::string& input,
                    char delemiter,
                    std::vector<double>* result) {
  std::vector<std::string> split;
  SplitString(input, delemiter, &split);
  double tmp;
  for (size_t i = 0; i < split.size(); ++i) {
    StringToDouble(split[i], &tmp);
    result->push_back(tmp);
  }
}

int StringToInt(const std::string& input) {
  int tmp;
  StringToInt(input, &tmp);
  return tmp;
}

int64 StringToInt64(const std::string& input) {
  int64 tmp;
  StringToInt64(input, &tmp);
  return tmp;
}

double StringToDouble(const std::string& input) {
  double tmp;
  StringToDouble(input, &tmp);
  return tmp;
}

static unsigned int strtoui(const char *nptr, char **endptr, int base) {
  uint64 res = strtoul(nptr, endptr, base);
#if __LP64__
  // Long is 64-bits, we have to handle under/overflow ourselves.  Test to see
  // if the result can fit into 32-bits (as signed or unsigned).
  if (static_cast<int>(static_cast<int32>(res)) != static_cast<int64>(res) &&
      static_cast<unsigned int>(res) != res) {
    res = kuint32max;
    errno = ERANGE;
  }
#endif
  return static_cast<unsigned int>(res);
}

class HexStringToIntTraits {
  public:
    typedef std::string string_type;
    typedef int value_type;
    static const int kBase = 16;
    static inline value_type convert_func(const string_type::value_type* str,
        string_type::value_type** endptr) {
      return strtoui(str, endptr, kBase);
    }
    static inline bool valid_func(const string_type& str) {
      return !str.empty() && !isspace(str[0]);
    }
};

bool HexStringToInt(const std::string& input, int* output) {
  return StringToNumber<HexStringToIntTraits>(input, output);
}
