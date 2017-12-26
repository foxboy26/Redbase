cc_library(
    name = "pf_page_handle",
    srcs = [
        "pf_page_handle.cc",
    ],
    hdrs = [
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
        ":pf_page_handle",
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
        ":pf_file_handle",
    ],
)
