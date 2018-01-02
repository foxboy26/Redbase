package(default_visibility = ["//visibility:public"])

cc_library(
    name = "pf_page_handle",
    srcs = [
        "pf_page_handle.cc",
    ],
    hdrs = [
        "pf.h",
        "pf_page_handle.h",
    ],
    deps = [
        "//src:rc",
    ],
)

cc_library(
    name = "pf_file_handle",
    srcs = [
        "pf_file_handle.cc",
    ],
    hdrs = [
        "pf_file_handle.h",
    ],
    deps = [
        ":pf_buffer_pool",
        ":pf_page_handle",
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "pf_file_handle_test",
    srcs = [
        "pf_file_handle_test.cc",
    ],
    deps = [
        ":pf_file_handle",
        ":pf_manager",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "pf_manager",
    srcs = [
        "pf_manager.cc",
    ],
    hdrs = [
        "pf_manager.h",
    ],
    deps = [
        ":pf_buffer_pool",
        ":pf_file_handle",
        "@com_google_glog//:glog",
    ],
)

cc_library(
    name = "pf_buffer_pool",
    srcs = [
        "pf_buffer_pool.cc",
    ],
    hdrs = [
        "pf.h",
        "pf_buffer_pool.h",
        "pf_internal.h",
    ],
    deps = [
        ":pf_page_handle",
        "//src:rc",
        "@com_google_glog//:glog",
    ],
)

cc_test(
    name = "pf_buffer_pool_test",
    srcs = [
        "pf_buffer_pool_test.cc",
    ],
    deps = [
        ":pf_buffer_pool",
        "@com_google_googletest//:gtest_main",
    ],
)
