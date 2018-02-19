package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "redbase",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":rc",
        "//src/pf:pf_file_handle",
        "//src/pf:pf_manager",
        "@com_github_gflags_gflags//:gflags",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "rc",
    #    srcs = [
    #        "rc.cc",
    #    ],
    hdrs = [
        "rc.h",
    ],
)
