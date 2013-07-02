#include <iostream>

using namespace std;

class Error {
public:
  virtual void PrintError() = 0;
};

class PF_Error : public Error {
public:
  void PrintError() {
    cout << "PF" << endl;
  }
};

class RM_Error : public Error {
public:
  void PrintError() {
    cout << "RM" << endl;
  }
};


void print(Error* e)
{
  e->PrintError();
}

PF_Error cmd()
{
  PF_Error pfError;

  return pfError;
}

int main()
{
  PF_Error pfError;
  RM_Error rmError;
  
  print(&pfError);

  print(&rmError);

  return 0;
}
