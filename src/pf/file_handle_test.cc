#include "file_handle.h"
#include "manager.h"

#include "absl/memory/memory.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

using redbase::RC;
using redbase::pf::PageNum;

#define EXPECT_OK(expr) EXPECT_THAT(expr, RC::OK)
#define ASSERT_OK(expr) ASSERT_THAT(expr, RC::OK)

namespace {
TEST(PF_FileHandleTest, OpenFile) {
  const char *filename = "test.rdb";
  redbase::pf::Manager pfm;
  ASSERT_OK(pfm.CreateFile(filename));

  auto bufferPool = absl::make_unique<redbase::pf::BufferPool>(5);
  redbase::pf::FileHandle fh(bufferPool.get());
  EXPECT_FALSE(fh.IsOpen());
  ASSERT_OK(fh.Open(filename));
  EXPECT_TRUE(fh.IsOpen());

  // Close the file.
  ASSERT_OK(fh.Close());
  EXPECT_FALSE(fh.IsOpen());

  // clean up.
  ASSERT_OK(pfm.DestroyFile(filename));
}

TEST(PF_FileHandleTest, OpenNonExistFile) {
  redbase::pf::FileHandle fh(nullptr);
  EXPECT_FALSE(fh.IsOpen());
  EXPECT_EQ(fh.Open("non_exist.rdb"), RC::PF_UNIX);
  EXPECT_FALSE(fh.IsOpen());
}
} // namespace

namespace {
class FileHandleTest : public ::testing::Test {
protected:
  FileHandleTest()
      : filename_("file_handle_test.rdb"),
        bufferPool_(absl::make_unique<redbase::pf::BufferPool>(5)) {}
  virtual ~FileHandleTest() = default;

  virtual void SetUp() override {
    ASSERT_OK(pfm_.CreateFile(filename_.c_str()));
    redbase::pf::FileHandle fh(bufferPool_.get());
    ASSERT_OK(fh.Open(filename_.c_str()));

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

    ASSERT_OK(fh.Close());
  }

  virtual void TearDown() override {
    ASSERT_OK(pfm_.DestroyFile(filename_.c_str()));
  }

  redbase::pf::Manager pfm_;
  std::string filename_;
  std::unique_ptr<redbase::pf::BufferPool> bufferPool_;
};

TEST_F(FileHandleTest, AllocateDisposePage) {
  redbase::pf::FileHandle fh(bufferPool_.get());
  ASSERT_OK(fh.Open(filename_.c_str()));

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

  ASSERT_OK(fh.Close());
}

TEST_F(FileHandleTest, AccessPage) {
  redbase::pf::FileHandle fh(bufferPool_.get());
  ASSERT_OK(fh.Open(filename_.c_str()));

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

  ASSERT_OK(fh.Close());
}

TEST_F(FileHandleTest, CloseShouldFlushBufferPool) {
  redbase::pf::FileHandle fh(bufferPool_.get());
  ASSERT_OK(fh.Open(filename_.c_str()));

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

  fh.Close();
}

} // namespace
