#include "common/flags.h"
#include "common/log.h"
#include "src/event_loop.h"
#include "src/event_handler.h"
#include "src/sock_event_handler.h"
#include "src/net.h"

DEFINE_string(pid_file, "tc.pid", "pid file name");
DEFINE_string(log_file, "tc.log", "");
DEFINE_bool(daemonize, false, "");

class SockAcceptHandler : public EventHandler {
 public:
  SockAcceptHandler() {}
  SockAcceptHandler(const shared_ptr<EventLoop>& el)
      : EventHandler(el) {}
  virtual ~SockAcceptHandler() {}
  virtual void Process(int fd,
                       IOMask mask,
                       void* client_data) {
    Log::WriteToDisk(DEBUG, "new conn comes!");
  }
 private:
  DO_NOT_COPY_AND_ASSIGN(SockAcceptHandler);
};

class ConnCheckHandler : public EventHandler {
 public:
  ConnCheckHandler() {}
  ConnCheckHandler(const shared_ptr<EventLoop>& el)
      : EventHandler(el) {}
  virtual ~ConnCheckHandler() {}
  virtual void Process(int fd,
                       IOMask mask,
                       void* client_data) {
    CheckConnection(fd);
    el_->DelEvent(fd, Write);

    shared_ptr<EventHandler> handler(
        new SockEventHandler(el_));
    if (el_->AddEvent(fd,
                      Read|Write,
                      handler,
                      handler,
                      NULL) == -1) {
      Log::WriteToDisk(ERROR, "add event fails.");
    }
  }
 private:
  DO_NOT_COPY_AND_ASSIGN(ConnCheckHandler);
};

#include <unistd.h>
int main(int argc, char* argv[]) {
  base::ParseCommandLineFlags(&argc, &argv, false);

  Log::Init(FLAGS_log_file, 0, 0);

  shared_ptr<EventLoop> el(new EventLoop("epoll"));

  int sock = CreateTcpClient("127.0.0.1", 6688);

  shared_ptr<EventHandler> handler(
      new ConnCheckHandler(el));
  if (el->AddEvent(sock, Write, handler, handler, NULL)) {
    Log::WriteToDisk(DEBUG, "Add checker.");
  }

  el->Start();

  el->Stop();

  return 0;
}
