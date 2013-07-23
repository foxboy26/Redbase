#include "PF_BufferPool.h"
#include "PF_PageHandle.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
using namespace std;

void genTestData(char* pData, char c);
int initTestFile(int fd);

int main()
{
  int fd;
  try
  {
    PF_BufferPool bufferPool(2);

    fd = open("test.dat", O_CREAT | O_RDWR, 0600);
    if (fd == -1)
    {
      throw PF_Exception(PF_Exception::UNIX);
    }

    initTestFile(fd);

    char* pData;

    bufferPool.GetPage(fd, 1, pData); 

    cout << bufferPool << endl;
    
    bufferPool.MarkDirty(fd, 1);

    pData[0] = 'd';
    cout << bufferPool << endl;

    bufferPool.UnpinPage(fd, 1);
    cout << bufferPool << endl;

    bufferPool.GetPage(fd, 2, pData); 

    cout << bufferPool << endl;
    
    bufferPool.MarkDirty(fd, 2);

    pData[0] = 'd';
    cout << bufferPool << endl;

    bufferPool.UnpinPage(fd, 2);
    cout << bufferPool << endl;

    bufferPool.GetPage(fd, 3, pData); 

    cout << bufferPool << endl;
    
    bufferPool.MarkDirty(fd, 3);

    pData[0] = 'd';
    cout << bufferPool << endl;

    bufferPool.UnpinPage(fd, 3);
    cout << bufferPool << endl;
    
  }
  catch (PF_Exception& e)
  {
    cerr << e.what() << endl;
  }

  if (close(fd) == -1)
  {

  }
  return 0;
}

void genTestData(char* pData, char c)
{
  memset(pData, c, PF_PAGE_SIZE);
}

int initTestFile(int fd)
{
  char pData[PF_PAGE_SIZE];

  for (int i = 0; i < 5; i++)
  {
    genTestData(pData, 'a' + i);
    ssize_t numBytes = write(fd, pData, PF_PAGE_SIZE);
  }
}
