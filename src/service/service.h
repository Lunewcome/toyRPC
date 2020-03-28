#ifndef SRC_SERVICE_SERVICE_H
#define SRC_SERVICE_SERVICE_H

#include "common/basics.h"

class Service {
 public:
  Service() {}
  virtual ~Service() {}
  virtual void AddMethod(const string& uri, const string& method_name) = 0;

 private:
  DO_NOT_COPY_AND_ASSIGN(Service);
};

#endif  // SRC_SERVICE_SERVICE_H
