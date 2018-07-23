package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "redbase",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":common",
        "//src/pf:file_handle",
        "//src/pf:manager",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "common",
    hdrs = [
        "common.h",
        "rc.h",
    ],
)
