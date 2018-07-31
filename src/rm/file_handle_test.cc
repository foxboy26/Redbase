#include "file_handle.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/common/test_utils.h"
#include "src/pf/file_handle.h"
#include "src/pf/manager.h"
#include "src/rc.h"
#include "src/rm/manager.h"
#include "src/rm/record.h"

using redbase::RC;
using redbase::rm::RID;
using redbase::rm::Record;

namespace redbase {
namespace rm {
constexpr int kRecordSize = 5;
constexpr int kBufferPoolSize = 5;

class RM_FileHandleTest : public ::testing::Test {
protected:
  RM_FileHandleTest()
      : filename_("file_handle_test.rdb"),
        pfm_(absl::make_unique<redbase::pf::Manager>(
            absl::make_unique<redbase::pf::BufferPool>(kBufferPoolSize))),
        rmm_(absl::make_unique<redbase::rm::Manager>(pfm_.get())) {}
  virtual ~RM_FileHandleTest() = default;

  virtual void SetUp() override {
    ASSERT_OK(rmm_->CreateFile(filename_, kRecordSize));
  }

  virtual void TearDown() override { ASSERT_OK(rmm_->DestroyFile(filename_)); }

  std::string filename_;
  std::unique_ptr<redbase::pf::Manager> pfm_;
  std::unique_ptr<redbase::rm::Manager> rmm_;
};

TEST_F(RM_FileHandleTest, InsertAndGet) {
  redbase::rm::FileHandle fh;
  ASSERT_OK(rmm_->OpenFile(filename_, &fh));

  for (int i = 0; i < 10; i++) {
    char data[kRecordSize];
    std::memset(data, static_cast<char>(i + 'a'), kRecordSize);
    RID rid;
    EXPECT_OK(fh.InsertRec(data, &rid));
    EXPECT_EQ(RID(1 /* page_num */, i /*slot_num*/), rid);
  }

  for (int i = 0; i < 10; i++) {
    RID rid(1 /* page_num */, i /*slot_num*/);
    Record record;
    EXPECT_OK(fh.GetRec(rid, &record));

    RID got_rid;
    EXPECT_OK(record.GetRid(&got_rid));
    EXPECT_EQ(rid, got_rid);

    char *data;
    EXPECT_OK(record.GetData(data));
    for (int j = 0; j < kRecordSize; j++) {
      EXPECT_EQ(static_cast<char>(i + 'a'), data[j]);
    }
  }

  ASSERT_OK(rmm_->CloseFile(&fh));
}

TEST_F(RM_FileHandleTest, Delete) {
  redbase::rm::FileHandle fh;
  ASSERT_OK(rmm_->OpenFile(filename_, &fh));

  for (int i = 0; i < 10; i++) {
    char data[kRecordSize];
    std::memset(data, static_cast<char>(i + 'a'), kRecordSize);
    RID rid;
    EXPECT_OK(fh.InsertRec(data, &rid));
    EXPECT_EQ(RID(1 /* page_num */, i /*slot_num*/), rid);
  }

  // Delete 0, 2, ... 8
  for (int i = 0; i < 5; i++) {
    EXPECT_OK(fh.DeleteRec(RID(1 /* page_num */, i * 2)));
  }

  for (int i = 0; i < 5; i++) {
    char data[kRecordSize];
    std::memset(data, static_cast<char>(i * 2 + 'A'), kRecordSize);
    RID rid;
    EXPECT_OK(fh.InsertRec(data, &rid));
    EXPECT_EQ(RID(1 /* page_num */, i * 2 /*slot_num*/), rid);
  }

  for (int i = 0; i < 5; i++) {
    RID rid(1 /* page_num */, i * 2 /*slot_num*/);
    Record record;
    EXPECT_OK(fh.GetRec(rid, &record));

    RID got_rid;
    EXPECT_OK(record.GetRid(&got_rid));
    EXPECT_EQ(rid, got_rid);

    char *data;
    EXPECT_OK(record.GetData(data));
    for (int j = 0; j < kRecordSize; j++) {
      EXPECT_EQ(static_cast<char>(i * 2 + 'A'), data[j]);
    }
  }
}

TEST_F(RM_FileHandleTest, Update) {
  redbase::rm::FileHandle fh;
  ASSERT_OK(rmm_->OpenFile(filename_, &fh));

  for (int i = 0; i < 10; i++) {
    char data[kRecordSize];
    std::memset(data, static_cast<char>(i + 'a'), kRecordSize);
    RID rid;
    EXPECT_OK(fh.InsertRec(data, &rid));
    EXPECT_EQ(RID(1 /* page_num */, i /*slot_num*/), rid);
  }

  for (int i = 0; i < 5; i++) {
    char data[kRecordSize];
    std::memset(data, static_cast<char>(i * 2 + 'A'), kRecordSize);
    Record record;
    record.Init(RID(1 /* page_num */, i * 2), data, kRecordSize);
    EXPECT_OK(fh.UpdateRec(record));
  }

  for (int i = 0; i < 5; i++) {
    RID rid(1 /* page_num */, i * 2 /*slot_num*/);
    Record record;
    EXPECT_OK(fh.GetRec(rid, &record));

    RID got_rid;
    EXPECT_OK(record.GetRid(&got_rid));
    EXPECT_EQ(rid, got_rid);

    char *data;
    EXPECT_OK(record.GetData(data));
    for (int j = 0; j < kRecordSize; j++) {
      EXPECT_EQ(static_cast<char>(i * 2 + 'A'), data[j]);
    }
  }
}

} // namespace rm
} // namespace redbase
