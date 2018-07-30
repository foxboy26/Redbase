#include "file_handle.h"

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/common/test_utils.h"
#include "src/pf/manager.h"

using redbase::RC;
using redbase::pf::PageNum;

namespace {
TEST(PF_FileHandleTest, OpenFile) {
  absl::string_view filename = "test.rdb";
  redbase::pf::Manager pfm(absl::make_unique<redbase::pf::BufferPool>(5));
  ASSERT_OK(pfm.CreateFile(filename));

  redbase::pf::FileHandle fh;
  ASSERT_OK(pfm.OpenFile(filename, &fh));
  EXPECT_TRUE(fh.IsOpen());

  // Close the file.
  ASSERT_OK(pfm.CloseFile(&fh));
  EXPECT_FALSE(fh.IsOpen());

  // clean up.
  ASSERT_OK(pfm.DestroyFile(filename));
}

TEST(PF_FileHandleTest, OpenNonExistFile) {
  redbase::pf::Manager pfm(absl::make_unique<redbase::pf::BufferPool>(5));
  redbase::pf::FileHandle fh;
  EXPECT_FALSE(fh.IsOpen());
  EXPECT_EQ(RC::PF_UNIX, pfm.OpenFile("non_exist.rdb", &fh));
  EXPECT_FALSE(fh.IsOpen());
}
} // namespace

namespace {
class FileHandleTest : public ::testing::Test {
protected:
  FileHandleTest()
      : pfm_(absl::make_unique<redbase::pf::BufferPool>(5)),
        filename_("file_handle_test.rdb") {}
  virtual ~FileHandleTest() = default;

  virtual void SetUp() override {
    ASSERT_OK(pfm_.CreateFile(filename_.c_str()));
    redbase::pf::FileHandle fh;
    ASSERT_OK(pfm_.OpenFile(filename_.c_str(), &fh));

    redbase::pf::PageHandle page;
    for (int i = 0; i < 10; i++) {
      ASSERT_OK(fh.AllocatePage(&page));
      EXPECT_EQ(page.GetPageNum(), i);
      ASSERT_OK(fh.UnpinPage(page.GetPageNum()));
      // check file header.
      // firstFree should always be LAST_FREE as we didn't delete any page.
      EXPECT_EQ(fh.GetFileHeader().firstFree, redbase::pf::LAST_FREE);
      // numPages should grow 1 by 1.
      EXPECT_EQ(fh.GetFileHeader().numPages, i + 1);
    }

    ASSERT_OK(pfm_.CloseFile(&fh));
    ASSERT_FALSE(fh.IsOpen());
  }

  virtual void TearDown() override {
    ASSERT_OK(pfm_.DestroyFile(filename_.c_str()));
  }

  redbase::pf::Manager pfm_;
  std::string filename_;
};

TEST_F(FileHandleTest, AllocateDisposePage) {
  redbase::pf::FileHandle fh;
  ASSERT_OK(pfm_.OpenFile(filename_.c_str(), &fh));

  // Remove 0, 2, 4, 6, 8
  for (int i = 0; i < 5; i++) {
    PageNum pageNum = i * 2;
    LOG(INFO) << "removing page: " << pageNum;
    ASSERT_OK(fh.DisposePage(pageNum));
    EXPECT_EQ(fh.GetFileHeader().firstFree, pageNum);
  }

  redbase::pf::PageHandle page;
  // Allocate should follow 8, 6, 4, 2, 0
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(fh.GetFileHeader().firstFree, 8 - i * 2);
    ASSERT_OK(fh.AllocatePage(&page));
    EXPECT_EQ(page.GetPageNum(), 8 - i * 2);
    ASSERT_OK(fh.UnpinPage(page.GetPageNum()));
  }

  ASSERT_OK(pfm_.CloseFile(&fh));
}

TEST_F(FileHandleTest, AccessPage) {
  redbase::pf::FileHandle fh;
  ASSERT_OK(pfm_.OpenFile(filename_.c_str(), &fh));

  // Remove 0, 2, 4, 6, 8
  for (int i = 0; i < 5; i++) {
    PageNum pageNum = i * 2;
    LOG(INFO) << "removing page: " << pageNum;
    ASSERT_OK(fh.DisposePage(pageNum));
    EXPECT_EQ(fh.GetFileHeader().firstFree, pageNum);
  }

  redbase::pf::PageHandle page;
  PageNum cur;

  // Verify GetFirstPage and GetNextPage
  {
    ASSERT_OK(fh.GetFirstPage(&page));
    cur = page.GetPageNum();
    EXPECT_EQ(cur, 1);
    ASSERT_OK(fh.UnpinPage(cur));

    int cnt = 0;
    RC rc = RC::OK;
    while (rc != RC::PF_EOF) {
      cnt++;
      cur = page.GetPageNum();
      rc = fh.GetNextPage(cur, &page);
      if (rc != RC::OK && rc != RC::PF_EOF) {
        LOG(ERROR) << "failed: rc " << rc;
        break;
      }
      fh.UnpinPage(cur);
    }
    EXPECT_EQ(cnt, 5);
  }

  // Verify GetLastPage and GetPrevPage
  {
    ASSERT_OK(fh.GetLastPage(&page));
    cur = page.GetPageNum();
    EXPECT_EQ(cur, 9);
    ASSERT_OK(fh.UnpinPage(cur));

    int cnt = 0;
    RC rc = RC::OK;
    while (rc != RC::PF_EOF) {
      cnt++;
      cur = page.GetPageNum();
      rc = fh.GetPrevPage(cur, &page);
      if (rc != RC::OK && rc != RC::PF_EOF) {
        LOG(ERROR) << "failed: rc " << rc;
        break;
      }
      fh.UnpinPage(cur);
    }
    EXPECT_EQ(cnt, 5);
  }

  ASSERT_OK(pfm_.CloseFile(&fh));
}

TEST_F(FileHandleTest, CloseShouldFlushBufferPool) {
  redbase::pf::FileHandle fh;
  ASSERT_OK(pfm_.OpenFile(filename_.c_str(), &fh));

  redbase::pf::PageHandle page;
  ASSERT_OK(fh.GetFirstPage(&page));
  char *pData;
  page.GetData(pData);
  ASSERT_TRUE(pData != nullptr);
  memset(pData, 'a', redbase::pf::PAGE_SIZE * sizeof(char));
  ASSERT_OK(fh.MarkDirty(page.GetPageNum()));
  ASSERT_OK(fh.UnpinPage(page.GetPageNum()));

  fh.Close();

  // reopen the file.
  ASSERT_OK(fh.Open(filename_.c_str()));
  ASSERT_OK(fh.GetFirstPage(&page));
  page.GetData(pData);
  ASSERT_TRUE(pData != nullptr);
  for (int i = 0; i < redbase::pf::PAGE_SIZE; i++) {
    EXPECT_EQ(pData[i], 'a');
  }
  ASSERT_OK(fh.UnpinPage(page.GetPageNum()));

  ASSERT_OK(pfm_.CloseFile(&fh));
}

} // namespace
