#include "src/rc.h"

typedef int PageNum;

struct PF_PageHeader {
  PageNum nextFree;
};

class PF_PageHandle {
 public:
  PF_PageHandle  (); // Default constructor
  ~PF_PageHandle (); // Destructor

  // non-copyable
  PF_PageHandle(const PF_PageHandle&) =delete;  
  PF_PageHandle& operator=(const PF_PageHandle&) =delete; 

  // GetPageNum returns the page number.
  PageNum GetPageNum() const;
  RC GetData(char *&pData) const;

 private:
  PageNum pageNum_;
  char* pData_;
};
