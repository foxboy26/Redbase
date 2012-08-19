#include "PF_HashTable.h"

#include <iostream>
#include <cassert>

using namespace std;

int main()
{
  cout << "==========Begin PF_HashTable test===========\n";

  PF_HashTable table(20);

  cout << "==========Test PF_HashTable::Insert=========\n";
  for (int i = 0; i < 40; i++)
  {
    assert(table.Insert(i, i + 10, i) == OK);
  }
  assert(table.Insert(0, 0 + 10, 0) == PF_HASHPAGEEXIST);
  assert(table.Insert(20, 20 + 10, 0) == PF_HASHPAGEEXIST);

  table.Print();

  cout << "==========Test PF_HashTable::Find===========\n";
  int slot;
  assert(table.Find(0, 0 + 10, slot) == OK);
  assert(table.Find(10, 10 + 10, slot) == OK);
  assert(table.Find(30, 30 + 10, slot) == OK);
  assert(table.Find(1, 0 + 10, slot) == PF_HASHNOTFOUND);
  assert(table.Find(4, 10 + 10, slot) == PF_HASHNOTFOUND);

  cout << "==========Test PF_HashTable::Delete=========\n";
  assert(table.Delete(0, 0 + 10) == OK);
  assert(table.Find(0, 0 + 10, slot) == PF_HASHNOTFOUND);
  for (int i = 1; i < 40; i++)
  {
    assert(table.Delete(i, i + 10) == OK);
  }

  table.Print();

  cout << "=========End of PF_HashTable test===========\n";

  return 0;
}
