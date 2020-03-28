cc_library (
  name = 'server',
  srcs = [
    'server.cc',
  ],
  deps = [
    '//common:base',
    '//src/net:net_util',
    '//src/protocol:proto_handler',
    '//src/reactor:reactor',
  ],
)

cc_binary (
  name = 'server_test',
  srcs = [
    'server_test.cc',
  ],
  deps = [
    '//common:base',
    '//src/server:server',
  ],
)
