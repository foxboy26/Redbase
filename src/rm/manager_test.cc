#include "manager.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using redbase::RC;
namespace {
TEST(ManagerTest, CreateAndDestroy) {
  redbase::pf::Manager pf_manager;
  redbase::pf::BufferPool buffer_pool(10);
  redbase::rm::Manager manager(&buffer_pool, &pf_manager);
  RC rc = manager.CreateFile("test.db", 10);
  EXPECT_EQ(RC::OK, rc);
  rc = manager.DestroyFile("test.db");
  EXPECT_EQ(RC::OK, rc);
}
} // namespace
