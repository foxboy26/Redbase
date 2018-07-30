#include "file_handle.h"

#include "absl/memory/memory.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/common/test_utils.h"
#include "src/pf/file_handle.h"
#include "src/pf/manager.h"
#include "src/rc.h"
#include "src/rm/manager.h"

using redbase::RC;
using redbase::rm::RID;
using redbase::rm::Record;

namespace {
constexpr int kRecordSize = 5;
constexpr int kBufferPoolSize = 5;

class FileHandleTest : public ::testing::Test {
protected:
  FileHandleTest()
      : filename_("file_handle_test.rdb"),
        pfm_(absl::make_unique<redbase::pf::Manager>(
            absl::make_unique<redbase::pf::BufferPool>(kBufferPoolSize))),
        rmm_(absl::make_unique<redbase::rm::Manager>(pfm_.get())) {}
  virtual ~FileHandleTest() = default;

  virtual void SetUp() override {
    LOG(INFO) << "HEHEH";
    ASSERT_OK(rmm_->CreateFile(filename_, kRecordSize));
  }

  virtual void TearDown() override {
    LOG(INFO) << "HEHEH";
    ASSERT_OK(rmm_->DestroyFile(filename_));
  }

  std::string filename_;
  std::unique_ptr<redbase::pf::Manager> pfm_;
  std::unique_ptr<redbase::rm::Manager> rmm_;
};

TEST_F(FileHandleTest, InsertAndGet) {
  redbase::rm::FileHandle fh;
  LOG(INFO) << "before open file";
  ASSERT_OK(rmm_->OpenFile(filename_, &fh));

  for (int i = 0; i < 10; i++) {
    char data[kRecordSize] = {static_cast<char>(i + 'a')};
    RID rid;
    EXPECT_OK(fh.InsertRec(data, &rid));
    EXPECT_EQ(RID(1 /* page_num */, i /*slot_num*/), rid);
  }

  ASSERT_OK(rmm_->CloseFile(&fh));
  LOG(INFO) << "after close file";
}

} // namespace
