package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "redbase",
    srcs = [
        "main.cc",
    ],
    deps = [
        ":rc",
    ],
)

cc_library(
    name = "rc",
    srcs = [
        "rc.cc",
    ],
    hdrs = [
        "rc.h",
    ],
)

cc_test(
    name = "rc_test",
    srcs = [
        "rc_test.cc",
    ],
    deps = [
        ":rc",
        "@com_google_googletest//:gtest_main",
    ],
)
