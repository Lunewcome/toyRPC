cc_binary (
  name = 'el',
  srcs = ['event_main.cc',
          'event_loop.cc',
          'epoll.cc'],
  deps = ['//common:base']
)

cc_binary (
  name = 'test_net_client',
  srcs = ['test_net_client.cc',
          'epoll.cc',
          'event_loop.cc',
          'net.cc'],
  deps = ['//common:base']
)
