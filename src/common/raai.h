#ifndef COMMON_RAAI_H
#define COMMON_RAAI_H

#include <functional>

#include "common/basics.h"

namespace base {

class RAAI {
 public:
  typedef std::function<void(void)> FType;
  RAAI(FType f) : f_(f) {}
  ~RAAI() {
   if (f_) {
    f_();
   }
  }
  void Release() { f_ = nullptr; }
 private:
  FType f_;

  BAN_COPY_AND_ASSIGN(RAAI);
};

}

#endif  // COMMON_RAAI_H
