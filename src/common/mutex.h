/** 
* @brief    Mutex wrapper.
* @date     2014-03-25
* @Authoer  XinlaiLu(luxinlai@baidu.com), Hi(newcome_lu)
*/

#ifndef BASE_MUTEX_H_
#define BASE_MUTEX_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

class CondVar;

class Mutex {
 public:
  enum LinkerInitialized { LINKER_INITIALIZED };
  Mutex() {
    if (pthread_mutex_init(&mu_, NULL) != 0) {
      printf("fail to init mutex!\n");
      abort();
    }
  }

  Mutex(LinkerInitialized) {
    if (pthread_mutex_init(&mu_, NULL) != 0) {
      printf("fail to init mutex!\n");
      abort();
    }
  }

  ~Mutex() {
    if (pthread_mutex_destroy(&mu_)) {
      printf("Fail to destroy mutex!\n");
    }
  }

  void Lock() {
    if (pthread_mutex_lock(&mu_) != 0) {
      printf("Fail to lock!\n");
      abort();
    }
  }

  void Unlock() {
    if (pthread_mutex_unlock(&mu_) != 0) {
      printf("Fail to unlock.\n");
      abort();
    }
  }

  pthread_mutex_t* GetMutex() {
    return &mu_;
  }

 private:
  pthread_mutex_t mu_;

  friend class CondVar;

  Mutex(const Mutex& obj);
  Mutex& operator=(const Mutex& obj);
};

class MutexLock {
 public:
  explicit MutexLock(Mutex* mu)
      : mu_(mu) {
    mu_->Lock();
  }

  ~MutexLock() {
    mu_->Unlock();
  }

 private:
  Mutex* mu_;

  MutexLock(const MutexLock& obj);
  MutexLock& operator=(const MutexLock& obj);
};

class CondVar {
 public:
  CondVar() {
    if (pthread_cond_init(&cond_var_, NULL) != 0) {
      printf("Fail to init cond_var!\n");
      abort();
    }
  }

  ~CondVar() {
    if (pthread_cond_destroy(&cond_var_) != 0) {
      printf("Fail to destroy cond_var!\n");
      abort();
    }
  }

  void Wait(Mutex* mu) {
    if (pthread_cond_wait(&cond_var_,
                          mu->GetMutex()) != 0) {
      printf("Failt to wait!\n");
      abort();
    }
  }

  void Signal() {
    if (pthread_cond_signal(&cond_var_) != 0) {
      printf("Fail to signal!\n");
      abort();
    }
  }

 private:
  pthread_cond_t cond_var_;

  CondVar(const CondVar& obj);
  CondVar& operator=(const CondVar& obj);
};

#endif  // BASE_MUTEX_H_
