/** 
* @brief    load conf
* @date     2014-03-17
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/
#ifndef BASE_STRING_UTIL_H_
#define BASE_STRING_UTIL_H_
#include "common/basictypes.h"

#include <stdarg.h>
#include <string>
#include <vector>

void StringAppendV(std::string* dst,
                   const char* format,
                   va_list ap);

void StringAppendF(std::string* dst,
                   const char* format,
                   ...);

void StringPrintf(std::string* dst,
                  const char* format,
                  ...);

bool StringToInt(const std::string& input, int* output);
int StringToInt(const std::string& input);

bool StringToInt64(const std::string& input, int64* output);
int64 StringToInt64(const std::string& input);

bool StringToDouble(const std::string& input, double* output);
double StringToDouble(const std::string& input);

void StringToDouble(const std::string& input,
                    char delemiter,
                    std::vector<double>* result);

void SplitString(const std::string& in_str,
                 char delemiter,
                 std::vector<std::string>* results);
void SplitString(const char* in_str,
                 char delemiter,
                 std::vector<std::string>* results);

bool HexStringToInt(const std::string& input, int* output);
#endif  // BASE_STRING_UTIL_H_
