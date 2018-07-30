#ifndef PF_FILEHANDLE_H
#define PF_FILEHANDLE_H

#include "absl/strings/string_view.h"

#include "src/pf/buffer_pool.h"
#include "src/pf/page_handle.h"

namespace redbase {
namespace pf {
struct FileHeader {
  PageNum firstPage;
  PageNum lastPage;
  PageNum firstFree;
  int numPages;

  FileHeader()
      : firstPage(-1), lastPage(-1), firstFree(LAST_FREE), numPages(0) {}
};

class FileHandle {
public:
  explicit FileHandle()
      : buffer_pool_(nullptr), header_(), fd_(-1), is_open_(false),
        is_header_modified(false) {}
  ~FileHandle(); // Destructor

  FileHandle(const FileHandle &fileHandle) = delete;
  FileHandle &operator=(const FileHandle &fileHandle) = delete;

  RC Open(absl::string_view file_name);
  RC Close();

  RC GetFirstPage(PageHandle *page_handle) const; // Get the first page
  RC GetLastPage(PageHandle *page_handle) const;  // Get the last page

  // Get the next page
  RC GetNextPage(PageNum current, PageHandle *page_handle) const;
  // Get the previous page
  RC GetPrevPage(PageNum current, PageHandle *page_handle) const;
  // Get a specific page
  RC GetThisPage(PageNum pageNum, PageHandle *page_handle) const;
  RC AllocatePage(PageHandle *page_handle);         // Allocate a new page
  RC DisposePage(PageNum pageNum);                  // Dispose of a page
  RC MarkDirty(PageNum pageNum) const;              // Mark a page as dirty
  RC UnpinPage(PageNum pageNum) const;              // Unpin a page
  RC ForcePages(PageNum pageNum = ALL_PAGES) const; // Write dirty page(s)
                                                    //   to disk
  bool IsOpen() const { return fd_ != -1 && is_open_; }

  const FileHeader &GetFileHeader() { return header_; }

private:
  explicit FileHandle(BufferPool *buffer_pool);

  RC ReadFileHeader();
  RC WriteFileHeader();
  RC AllocateNewPage(PageNum pageNum);
  bool IsValidPageNum(PageNum pageNum) const {
    return (pageNum >= 0 && pageNum <= header_.numPages);
  }

  BufferPool *buffer_pool_ = nullptr; // not owned.
  FileHeader header_;
  int fd_;
  bool is_open_;
  bool is_header_modified;

  friend class Manager;
};
} // namespace pf
} // namespace redbase

#endif // PF_FILEHANDLE_H
