#include "pf_file_handle.h"
#include "pf_manager.h"

#include "absl/memory/memory.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

#define EXPECT_OK(expr) EXPECT_THAT(expr, RC::OK)
#define ASSERT_OK(expr) ASSERT_THAT(expr, RC::OK)

namespace {
TEST(PF_FileHandleTest, OpenFile) {
  const char *filename = "test.rdb";
  PF_Manager pfm;
  ASSERT_OK(pfm.CreateFile(filename));

  PF_FileHandle fh(nullptr);
  EXPECT_FALSE(fh.IsOpen());
  ASSERT_OK(fh.Open(filename));
  EXPECT_TRUE(fh.IsOpen());

  // Close the file.
  ASSERT_OK(fh.Close());
  EXPECT_FALSE(fh.IsOpen());

  ASSERT_OK(pfm.DestroyFile(filename));
}

TEST(PF_FileHandleTest, OpenNonExistFile) {
  PF_FileHandle fh(nullptr);
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
        bufferPool_(absl::make_unique<PF_BufferPool>(5)) {}
  virtual ~FileHandleTest() = default;

  virtual void SetUp() override {
    ASSERT_OK(pfm_.CreateFile(filename_.c_str()));
    PF_FileHandle fh(bufferPool_.get());
    ASSERT_OK(fh.Open(filename_.c_str()));

    PF_PageHandle page;
    for (int i = 0; i < 10; i++) {
      ASSERT_OK(fh.AllocatePage(&page));
      EXPECT_EQ(page.GetPageNum(), i);
      ASSERT_OK(fh.UnpinPage(page.GetPageNum()));
      // check file header.
      // firstFree should always be LAST_FREE as we didn't delete any page.
      EXPECT_EQ(fh.FileHeader().firstFree, LAST_FREE);
      // numPages should grow 1 by 1.
      EXPECT_EQ(fh.FileHeader().numPages, i + 1);
    }

    ASSERT_OK(fh.Close());
  }

  virtual void TearDown() override {
    ASSERT_OK(pfm_.DestroyFile(filename_.c_str()));
  }

  PF_Manager pfm_;
  std::string filename_;
  std::unique_ptr<PF_BufferPool> bufferPool_;
};

TEST_F(FileHandleTest, AllocateDisposePage) {
  PF_FileHandle fh(bufferPool_.get());
  ASSERT_OK(fh.Open(filename_.c_str()));

  // Remove 0, 2, 4, 6, 8
  for (int i = 0; i < 5; i++) {
    PageNum pageNum = i * 2;
    LOG(INFO) << "removing page: " << pageNum;
    ASSERT_OK(fh.DisposePage(pageNum));
    EXPECT_EQ(fh.FileHeader().firstFree, pageNum);
  }

  PF_PageHandle page;
  // Allocate should follow 8, 6, 4, 2, 0
  for (int i = 0; i < 5; i++) {
    EXPECT_EQ(fh.FileHeader().firstFree, 8 - i * 2);
    ASSERT_OK(fh.AllocatePage(&page));
    EXPECT_EQ(page.GetPageNum(), 8 - i * 2);
    ASSERT_OK(fh.UnpinPage(page.GetPageNum()));
  }

  ASSERT_OK(fh.Close());
}

TEST_F(FileHandleTest, AccessPage) {
  PF_FileHandle fh(bufferPool_.get());
  ASSERT_OK(fh.Open(filename_.c_str()));

  // Remove 0, 2, 4, 6, 8
  for (int i = 0; i < 5; i++) {
    PageNum pageNum = i * 2;
    LOG(INFO) << "removing page: " << pageNum;
    ASSERT_OK(fh.DisposePage(pageNum));
    EXPECT_EQ(fh.FileHeader().firstFree, pageNum);
  }

  PF_PageHandle page;
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

} // namespace
