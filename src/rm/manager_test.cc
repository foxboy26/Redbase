#include "manager.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using redbase::RC;
namespace {
TEST(ManagerTest, CreateAndDestroy) {
  redbase::pf::Manager pf_manager(
      absl::make_unique<redbase::pf::BufferPool>(10));
  redbase::rm::Manager manager(&pf_manager);
  RC rc = manager.CreateFile("test.db", 10);
  EXPECT_EQ(RC::OK, rc);
  rc = manager.DestroyFile("test.db");
  EXPECT_EQ(RC::OK, rc);
}
} // namespace
