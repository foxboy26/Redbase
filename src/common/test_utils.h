#ifndef COMMON_TEST_UTILS_H
#define COMMON_TEST_UTILS_H

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define EXPECT_OK(expr) EXPECT_THAT(expr, redbase::RC::OK)
#define ASSERT_OK(expr) ASSERT_THAT(expr, redbase::RC::OK)

#endif // COMMON_TEST_UTILS_H
