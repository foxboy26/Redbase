#ifndef PF_PF_FILEHANDLE_H
#define PF_PF_FILEHANDLE_H

#include "src/pf/pf_page_handle.h"
#include "src/rc.h"

class PF_FileHandle {
public:
  PF_FileHandle();  // Default constructor
  ~PF_FileHandle(); // Destructor

  PF_FileHandle(const PF_FileHandle &fileHandle) = delete;
  PF_FileHandle &operator=(const PF_FileHandle &fileHandle) = delete;
  // Overload =
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
private:
  // a list of PageHandle
};

#endif // PF_PF_FILEHANDLE_H
