load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_proto_library", "objc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

COPTS = [
  "-g"
]

cc_library (
  name = "common",
  includes = [
    "src/",
  ],
  hdrs = glob([
    "src/common/*.h",
  ]),
  srcs = glob([
    "src/common/*.cc",
  ]),
  deps = [
    "@com_github_google_glog//:glog",
  ],
)

cc_library (
  name = "toyRPC",
  includes = [
    "src/",
  ],
  hdrs = glob([
    "src/*.h",
    "src/protocols/*.h",
  ]),
  srcs = glob([
    "src/*.cc",
    "src/protocols/*.cc",
  ]),
  deps = [
    ":common",
    "@com_github_gflags_gflags//:gflags",
    "@com_github_google_glog//:glog",
    "@com_google_protobuf//:protobuf",
    #"@com_google_googletest//:gtest_main",
  ],
  copts = COPTS,
  visibility = ["//visibility:public"],
)

cc_binary (
  name = "ExampleServer",
  srcs = [
    "examples/example_server.cc",
  ],
  deps = [
    "toyRPC",
  ],
)

cc_binary (
  name = "ExampleClient",
  srcs = [
    "examples/example_client.cc",
  ],
  deps = [
    "toyRPC",
  ],
)

proto_library (
  name = "rpc_protocol_proto",
  srcs = [
    "src/protocol.proto",
  ],
)

cc_proto_library (
  name = "cc_rpc_protocol",
  deps = [
    ":rpc_protocol_proto",
  ]
)
