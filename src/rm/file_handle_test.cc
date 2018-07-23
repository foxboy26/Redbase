#include "file_handle.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using redbase::RC;
using redbase::rm::RID;
using redbase::rm::Record;

namespace {
GTEST_TEST(RM_RIDTest, Get) {
  RID rid;
  EXPECT_EQ(redbase::pf::kInvalidPageNum, rid.GetPageNum());
  EXPECT_EQ(redbase::rm::kInvalidSlotNum, rid.GetSlotNum());
}

TEST(RM_RIDTest, IsValid) {
  RID rid;
  EXPECT_FALSE(rid.IsValid());

  rid = RID(1, 1);
  EXPECT_TRUE(rid.IsValid());
}

TEST(RM_RecordTest, Get) {
  Record record;
  char *pData;
  EXPECT_EQ(RC::RM_INVALID_RECORD, record.GetData(pData));
  RID rid;
  EXPECT_EQ(RC::RM_INVALID_RECORD, record.GetRid(&rid));
}

} // namespace
