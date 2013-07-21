#include "PF_PageHandle.h"

#include <iostream>
using namespace std;

int main()
{
  PF_PageHandle pageHandle;

  try {
    pageHandle.GetData();
  }
  catch (const Exception& e)
  {
    cerr << e.what() << endl;
  }

  return 0;
}
