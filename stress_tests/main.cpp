//
// Created by gogagum on 10.08.2020.
//

#include "insert_stress.cpp"
#include "extract_stress.cpp"
#include "set_stress.cpp"

void RunAllTests() {
  RunInsertsTestPack();
  //RunExtractsTestPack();
  //RunSetsTestPack();
}

int main() {
  RunAllTests();
  return 0;
}