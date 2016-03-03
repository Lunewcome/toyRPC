/**
 */

#include "common/flags.h"
#include "common/log.h"
#include "src/event_loop.h"

#include <iostream>
using namespace std;

DEFINE_string(pid_file, "el.pid", "pid file name");
DEFINE_string(log_file, "el.log", "");
DEFINE_bool(daemonize, true, "");
DEFINE_int32(v, 0, "");

int main(int argc, char** argv) {
  base::ParseCommandLineFlags(&argc, &argv, false);

  Log::Init(FLAGS_log_file, 0, 0);
  EventLoop el;
  el.Start();
  el.Stop();

  return 0;
}
