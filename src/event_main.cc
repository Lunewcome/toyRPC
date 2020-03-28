/**
 */

#include "common/flags.h"
#include "glog/logging.h"
#include "src/reactor/event_loop.h"

DEFINE_string(pid_file, "el.pid", "pid file name");
DEFINE_string(log_file, "el.log", "");
DEFINE_bool(daemonize, true, "");

int main(int argc, char** argv) {
  base::ParseCommandLineFlags(&argc, &argv, false);

  EventLoop el;
  el.Start();
  el.Stop();

  return 0;
}
