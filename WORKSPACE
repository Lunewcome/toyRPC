workspace(name = "toyRPC")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

RULES_CC_SHA256 = "04d22a8c6f0caab1466ff9ae8577dbd12a0c7d0bc468425b75de094ec68ab4f9"
RULES_CC_COMMIT_ID = "0913abc3be0edff60af681c0473518f51fb9eeef"  # 2021-08-12T14:14:28Z
RULES_PROTO_TAG = "4.0.0"  # 2021-09-15T14:13:21Z
RULES_PROTO_SHA256 = "66bfdf8782796239d3875d37e7de19b1d94301e8972b3cbd2446b332429b4df1"

BAZEL_SKYLIB_VERSION = "1.1.1"  # 2021-09-27T17:33:49Z
BAZEL_SKYLIB_SHA256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d"

http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "122fb6b712808ef43fbf80f75c52a21c9760683dae470154f02bddfc61135022",
    strip_prefix = "glog-0.6.0",
    urls = ["https://github.com/google/glog/archive/v0.6.0.zip"],
)

http_archive(
    name = "rules_cc",
    sha256 = RULES_CC_SHA256,
    strip_prefix = "rules_cc-{commit_id}".format(commit_id = RULES_CC_COMMIT_ID),
    urls = [
        "https://github.com/bazelbuild/rules_cc/archive/{commit_id}.tar.gz".format(commit_id = RULES_CC_COMMIT_ID),
    ],
)

http_archive(
    name = "rules_proto",
    sha256 = RULES_PROTO_SHA256,
    strip_prefix = "rules_proto-{version}".format(version = RULES_PROTO_TAG),
    urls = ["https://github.com/bazelbuild/rules_proto/archive/refs/tags/{version}.tar.gz".format(version = RULES_PROTO_TAG)],
)

http_archive(
    name = "com_google_protobuf",  # 2021-10-29T00:04:02Z
    build_file = "//bazel/third_party/protobuf:protobuf.BUILD",
    patch_cmds = [
        "sed -i protobuf.bzl -re '4,4d;417,508d'",
    ],
    sha256 = "87407cd28e7a9c95d9f61a098a53cf031109d451a7763e7dd1253abf8b4df422",
    strip_prefix = "protobuf-3.19.1",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/refs/tags/v3.19.1.tar.gz"],
 )

http_archive(
  name = "com_google_googletest",
  urls = ["https://github.com/google/googletest/archive/5ab508a01f9eb089207ee87fd547d290da39d015.zip"],
  strip_prefix = "googletest-5ab508a01f9eb089207ee87fd547d290da39d015",
)

http_archive(
    name = "bazel_skylib",
    sha256 = BAZEL_SKYLIB_SHA256,
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
    ],
)


http_archive(
    name = "com_github_madler_zlib",  # 2017-01-15T17:57:23Z
    build_file = "//bazel/third_party/zlib:zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = [
        "https://downloads.sourceforge.net/project/libpng/zlib/1.2.11/zlib-1.2.11.tar.gz",
        "https://zlib.net/fossils/zlib-1.2.11.tar.gz",
    ],
)

http_archive(
    name = "com_github_nelhage_rules_boost",  # 2021-08-27T15:46:06Z
    patch_cmds = ["sed -i 's/net_zlib_zlib/com_github_madler_zlib/g' BUILD.boost"],
    patch_cmds_win = [
        """$content = (Get-Content 'BUILD.boost') -replace "net_zlib_zlib", "com_github_madler_zlib"
Set-Content BUILD.boost -Value $content -Encoding UTF8
""" ,
    ],
    sha256 = "2d0b2eef7137730dbbb180397fe9c3d601f8f25950c43222cb3ee85256a21869",
    strip_prefix = "rules_boost-fce83babe3f6287bccb45d2df013a309fa3194b8",
    urls = [
        "https://github.com/nelhage/rules_boost/archive/fce83babe3f6287bccb45d2df013a309fa3194b8.tar.gz",
    ],
)
