#include <fstream>
#include <iostream>

#include "src/rc.h"

int main(int argc, char *argv[]) {
  std::cout << "Welcome to Redbase" << std::endl;

  std::cout << "Bye!" << std::endl;

  PF_PrintError(RC::RC_EOF);

  return 0;
}
