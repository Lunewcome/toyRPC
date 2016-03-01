/**
 * log class.
 * not thread safe.
 */
#ifndef BASE_LOG_H_
#define BASE_LOG_H_

#include "common/basics.h"
#include <pthread.h>

#include <map>
#include <string>
using std::map;
using std::string;

typedef enum Level {
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3,
  FLUSH = 4,
  ALL,
} Level;

class Log {
 public:
  Log();
  ~Log();
  static void Init(const string& file_path,
                   int v,
                   int thread_safe);
  static void WriteToDisk(Level v, const string& msg);
  static void WriteToDisk(Level v,
                          const char* format,
                          ...);
  static void WriteToBuffer(Level v, const string& msg);
  static void WriteToBuffer(Level v,
                            const char* format,
                            ...);
  static void Flush();
  static void PrintToConsole(const string& msg);

 private:
  static void FormatMsg(const string& msg,
                        string* formated_msg);
  static void WriteToDiskInternal(
      Level v,
      const string& msg,
      bool formated = false);
  static void Lock();
  static void UnLock();

  static int fd_;
  static Level v_;
  static string buf_;
  static int thread_safe_;
  static pthread_mutex_t mu_;

  DO_NOT_COPY_AND_ASSIGN(Log);
};

#endif  // BASE_LOG_H_
