/** 
* @brief    singleton.
* @date     2014-04-09
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/
#ifndef COMMON_SINGLETON_H_
#define COMMON_SINGLETON_H_

#include "common/basics.h"
#include "common/mutex.h"
#include "common/shared_ptr.h"

template<class T>
class Singleton {
 public:
  static T* GetInstance() {
    if (!inst_.get()) {
      MutexLock lock(&m_);
      if (!inst_.get()) {
        inst_.reset(new T());
      }
    }
    return inst_.get();
  }

 private:
  Singleton();
  ~Singleton();

  static Mutex m_;
  static shared_ptr<T> inst_;

  DO_NOT_COPY_AND_ASSIGN(Singleton);
};
template<class T> Mutex Singleton<T>::m_;
template<class T> shared_ptr<T> Singleton<T>::inst_;

#endif  // COMMON_SINGLETON_H_
