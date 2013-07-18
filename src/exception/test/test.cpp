#include "PF_Exception.h"

#include <iostream>
using namespace std;

int main()
{
  try {
    throw PF_Exception(PF_Exception::PAGEUNPINNED);
  }
  catch (PF_Exception e) {
    cerr << e.what() << endl;
  }

  return 0;
}
