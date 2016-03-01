#include "common/log.h"

#include "common/flags.h"
#include "common/string_util.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <iostream>

#include <cstdlib>
#include <ctime>

int Log::fd_ = -1;
Level Log::v_ = DEBUG;
string Log::buf_ = "";
int Log::thread_safe_ = 0;
pthread_mutex_t Log::mu_;

Level levels[ALL + 1] = {
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FLUSH,
  ALL,
};

void Log::Init(const string& file_path,
               int v,
               int thread_safe) {
  if (fd_ != -1) {
    close(fd_);
  }
  fd_ = open(file_path.c_str(),
             O_APPEND|O_CREAT|O_RDWR,
             S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
  if (fd_ < 0) {
    abort();
  }
  if (v >= ERROR) {
    v = ERROR;
  }
  if (v < DEBUG) {
    v = DEBUG;
  }
  v_ = levels[v];
  thread_safe_ = thread_safe;
  if (thread_safe_) {
    pthread_mutex_init(&mu_, NULL);
  }
  atexit(Flush);
}

Log::~Log() {
  if (fd_ > 0) {
    close(fd_);
  }
}

void Log::WriteToDisk(Level v,
                      const char* format,
                      ...) {
  string tmp_msg;
  va_list ap;
  va_start(ap, format);
  StringAppendV(&tmp_msg, format, ap);
  va_end(ap);
  WriteToDisk(v, tmp_msg);
}

void Log::WriteToDisk(Level v, const string& msg) {
  Lock();
  WriteToDiskInternal(v, msg, false);
  UnLock();
}

void Log::WriteToBuffer(Level v,
                        const char* format,
                        ...) {
  string tmp_msg;
  va_list ap;
  va_start(ap, format);
  StringAppendV(&tmp_msg, format, ap);
  va_end(ap);
  WriteToBuffer(v, tmp_msg);
}

void Log::WriteToBuffer(Level v, const string& msg) {
  if (v < v_) {
    return;
  }
  Lock();
  string new_msg;
  FormatMsg(msg, &new_msg);
  buf_ += new_msg;
  UnLock();
  if (buf_.size() > 4 * 1024 || v_ <= INFO) {
    Flush();
  }
}

void Log::Flush() {
  Lock();
  WriteToDiskInternal(FLUSH, buf_, true);
  buf_.clear();
  UnLock();
}

void Log::WriteToDiskInternal(
    Level v,
    const string& msg,
    bool formated) {
  if (v < v_) {
    return;
  }
  if (!formated) {
    string new_msg;
    FormatMsg(msg, &new_msg);
    write(fd_, new_msg.c_str(), new_msg.size());
  } else {
    write(fd_, msg.c_str(), msg.size());
  }
}

void Log::Lock() {
  if (thread_safe_) {
    pthread_mutex_lock(&mu_);
  }
}

void Log::UnLock() {
  if (thread_safe_) {
    pthread_mutex_unlock(&mu_);
  }
}

void Log::FormatMsg(const string& msg,
                    string* formated_msg) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  static const int max_len = 100;
  char buf[max_len];
  strftime(buf,
           sizeof(buf),
           "%Y-%m-%d %H:%M:%S",
           localtime(&tv.tv_sec));
  StringPrintf(formated_msg,
               "[%s]%s\n",
               buf,
               msg.c_str());
}

void Log::PrintToConsole(const string& msg) {
  std::cout << msg << std::endl;
}
