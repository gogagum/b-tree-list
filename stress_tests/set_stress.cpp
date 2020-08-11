//
// Created by gogagum on 10.08.2020.
//

#include <algorithm>
#include <fstream>
#include <cstddef>
#include <chrono>
#include <string>
#include <boost/random.hpp>
#include "../lib/b_tree_list.hpp"

#ifndef SET_STRESS_CPP
#define SET_STRESS_CPP

auto TestSets(size_t size_of_btree, size_t inserts_cnt) {
  std::string file_name = "inserts_stress_test_data";
  auto* test_list = new BTreeList<int, 200>(file_name, size_of_btree);

  std::vector<unsigned> positions;
  for (unsigned i = 0; i < inserts_cnt; ++i) {
    positions.push_back(
        boost::minstd_rand(static_cast<unsigned>(std::time(nullptr)))() % inserts_cnt
    );
  }

  auto start = std::chrono::high_resolution_clock::now();
  for(auto i: positions) {
    test_list->Set(i, 0);
  }
  auto end = std::chrono::high_resolution_clock::now();

  delete test_list;
  boost::filesystem::remove(file_name);

  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

void RunSetsTestPack() {
  const size_t step = 10;
  const size_t max_size = 10000000;

  for (size_t size_of_btree = step; size_of_btree *= step; size_of_btree < max_size + 1) {
    std::string results_file_name = "sets_test_" + std::to_string(size_of_btree) + ".txt";
    std::ofstream file;
    file.open(results_file_name, std::ios_base::trunc);
    for (unsigned i = 0; i < 100; ++i) {
      auto inserts_cnt = std::min(size_of_btree / step, static_cast<size_t>(10000));
      file << TestSets(size_of_btree, inserts_cnt) / inserts_cnt << std::endl;
    }
    file.close();
  }
}

#endif