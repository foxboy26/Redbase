#ifndef PF_PF_FILEHANDLE_H
#define PF_PF_FILEHANDLE_H

#include "src/pf/pf_buffer_pool.h"
#include "src/pf/pf_page_handle.h"
#include "src/rc.h"

struct PF_FileHeader {
  PageNum firstPage;
  PageNum lastPage;
  int numPages;

  PF_FileHeader() : firstPage(-1), lastPage(-1), numPages(0) {}
};

class PF_FileHandle {
public:
  PF_FileHandle();  // Default constructor
  ~PF_FileHandle(); // Destructor

  PF_FileHandle(const PF_FileHandle &fileHandle) = delete;
  PF_FileHandle &operator=(const PF_FileHandle &fileHandle) = delete;

  RC Open(const char *filename);
  RC Close();

  RC GetFirstPage(PF_PageHandle *pageHandle) const; // Get the first page
  RC GetLastPage(PF_PageHandle *pageHandle) const;  // Get the last page

  RC GetNextPage(PageNum current, PF_PageHandle *pageHandle) const;
  // Get the next page
  RC GetPrevPage(PageNum current, PF_PageHandle *pageHandle) const;
  // Get the previous page
  RC GetThisPage(PageNum pageNum, PF_PageHandle *pageHandle) const;
  // Get a specific page
  RC AllocatePage(PF_PageHandle *pageHandle);       // Allocate a new page
  RC DisposePage(PageNum pageNum);                  // Dispose of a page
  RC MarkDirty(PageNum pageNum) const;              // Mark a page as dirty
  RC UnpinPage(PageNum pageNum) const;              // Unpin a page
  RC ForcePages(PageNum pageNum = ALL_PAGES) const; // Write dirty page(s)
                                                    //   to disk
  bool IsOpen() { return fd_ != -1 && isOpen_; }

private:
  RC ReadFileHeader();
  RC WriteFileHeader();

  PF_BufferPool *bufferPool_; // not owned.
  PF_FileHeader header_;
  int fd_;
  bool isOpen_;
  bool isHeadModfied_;
};

#endif // PF_PF_FILEHANDLE_H
