//
// Created by gogagum on 10.08.2020.
//

#include "insert_stress.cpp"
#include "extract_stress.cpp"
#include "set_stress.cpp"

void RunAllTests() {
  size_t max_size = 100000000;
  size_t operations_cnt = 100000;
  RunInsertsTestPack(max_size, operations_cnt);
  RunExtractsTestPack(max_size, operations_cnt);
  RunSetsTestPack(max_size, operations_cnt);
}

int main() {
  RunAllTests();
  return 0;
}