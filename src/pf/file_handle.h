#ifndef PF_FILEHANDLE_H
#define PF_FILEHANDLE_H

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
  explicit FileHandle(BufferPool *bufferPool);
  ~FileHandle(); // Destructor

  FileHandle(const FileHandle &fileHandle) = delete;
  FileHandle &operator=(const FileHandle &fileHandle) = delete;

  RC Open(const char *filename);
  RC Close();

  RC GetFirstPage(PageHandle *pageHandle) const; // Get the first page
  RC GetLastPage(PageHandle *pageHandle) const;  // Get the last page

  // Get the next page
  RC GetNextPage(PageNum current, PageHandle *pageHandle) const;
  // Get the previous page
  RC GetPrevPage(PageNum current, PageHandle *pageHandle) const;
  // Get a specific page
  RC GetThisPage(PageNum pageNum, PageHandle *pageHandle) const;
  RC AllocatePage(PageHandle *pageHandle);          // Allocate a new page
  RC DisposePage(PageNum pageNum);                  // Dispose of a page
  RC MarkDirty(PageNum pageNum) const;              // Mark a page as dirty
  RC UnpinPage(PageNum pageNum) const;              // Unpin a page
  RC ForcePages(PageNum pageNum = ALL_PAGES) const; // Write dirty page(s)
                                                    //   to disk
  bool IsOpen() const { return fd_ != -1 && isOpen_; }

  const FileHeader &GetFileHeader() { return header_; }

private:
  RC ReadFileHeader();
  RC WriteFileHeader();
  RC AllocateNewPage(PageNum pageNum);
  bool IsValidPageNum(PageNum pageNum) const {
    return (pageNum >= 0 && pageNum <= header_.numPages);
  }

  BufferPool *bufferPool_; // not owned.
  FileHeader header_;
  int fd_;
  bool isOpen_;
  bool isHeaderModified_;
};
} // namespace pf
} // namespace redbase

#endif // PF_FILEHANDLE_H
