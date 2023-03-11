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
