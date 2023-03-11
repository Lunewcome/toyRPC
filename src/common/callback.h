#ifndef COMMON_CALLBACK_H_
#define COMMON_CALLBACK_H_

#include "common/basics.h"

template<class Arg1>
class Closure {
 public:
  Closure() {}
  virtual ~Closure() {}
  virtual void Run(const Arg1&) = 0;
 private:
  DO_NOT_COPY_AND_ASSIGN(Closure);
};

template<bool del, class T, class Arg1>
class Closure_Param_1 : public Closure<Arg1> {
 public:
  typedef Closure<Arg1> Base;
  typedef void (T::*Function)(Arg1);
  Closure_Param_1(T* obj, Function func)
      : obj_(obj), func_(func) {}
  ~Closure_Param_1() {}
  virtual void Run(const Arg1& arg) {
    obj_->*func_(arg);
    if (del) {
      delete this;
    }
  }
 private:
  T* obj_;
  Function func_;

  DO_NOT_COPY_AND_ASSIGN(Closure_Param_1);
};

template<class T, class Arg1>
typename Closure_Param_1<false, T, Arg1>::Base*
NewLongLiveCallback(T* obj, void (T::*func)(Arg1)) {
  return new Closure_Param_1<false, T, Arg1>(obj, func);
}

#endif  // COMMON_CALLBACK_H_
