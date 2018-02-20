package(default_visibility = ["//visibility:public"])

cc_library(
    name = "rm",
    hdrs = ["rm.h"],
)

cc_library(
    name = "record",
    srcs = [
        #    "record.cc",
    ],
    hdrs = [
        "record.h",
    ],
    deps = [
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "file_handle",
    srcs = [
        "file_handle.cc",
    ],
    hdrs = [
        "file_handle.h",
    ],
    deps = [
        ":internal",
        ":record",
        ":rm",
        "//src/pf:file_handle",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "file_scan",
    srcs = [
        "file_scan.cc",
    ],
    hdrs = [
        "file_scan.h",
    ],
    deps = [
        ":file_handle",
        ":record",
        "//src:common",
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "file_handle_test",
    srcs = [
        "file_handle_test.cc",
    ],
    deps = [
        ":file_handle",
        ":manager",
        "@com_google_absl//absl/memory",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "internal",
    srcs = ["internal.cc"],
    hdrs = ["internal.h"],
    deps = [
        "//src/pf:internal",
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "internal_test",
    srcs = ["internal_test.cc"],
    deps = [
        ":internal",
        "@com_google_glog//:glog",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "manager",
    srcs = [
        "manager.cc",
    ],
    hdrs = [
        "manager.h",
    ],
    deps = [
        ":file_handle",
        ":internal",
        ":rm",
        "//src/pf:manager",
        "@com_google_glog//:glog",
    ],
)
