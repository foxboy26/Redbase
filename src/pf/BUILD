package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pf",
    hdrs = ["pf.h"],
)

cc_library(
    name = "page_handle",
    srcs = [
        "page_handle.cc",
    ],
    hdrs = [
        "page_handle.h",
        "pf.h",
    ],
    deps = [
        "//src:common",
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
        ":buffer_pool",
        ":page_handle",
        "@com_google_absl//absl/strings",
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
        "//src/common:test_utils",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
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
        ":buffer_pool",
        ":file_handle",
        "@com_google_absl//absl/strings",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "buffer_pool",
    srcs = [
        "buffer_pool.cc",
    ],
    hdrs = [
        "buffer_pool.h",
        "pf.h",
    ],
    deps = [
        ":internal",
        ":lru_cache",
        ":page_handle",
        "//src:common",
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "buffer_pool_test",
    srcs = [
        "buffer_pool_test.cc",
    ],
    deps = [
        ":buffer_pool",
        "//src/common:test_utils",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lru_cache",
    srcs = [
    ],
    hdrs = [
        "lru_cache.cc",
        "lru_cache.h",
    ],
    deps = [
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "lru_cache_test",
    srcs = [
        "lru_cache_test.cc",
    ],
    deps = [
        ":lru_cache",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "internal",
    hdrs = ["internal.h"],
    deps = ["//src/pf"],
)
