//
// Created by gogagum on 03.08.2020.
//

#include <cstddef>
#include <cstdint>

#ifndef B_TREE_LIST_LIB__DATA_INFO_HPP_
#define B_TREE_LIST_LIB__DATA_INFO_HPP_

typedef uint64_t file_pos_t;
typedef int64_t signed_file_pos_t;

struct DataInfo{
  signed_file_pos_t _stack_head_pos;
  file_pos_t _free_tail_start;
  file_pos_t _max_blocks_cnt;
  file_pos_t _root_pos;
  size_t _size;
};

#endif //B_TREE_LIST_LIB__DATA_INFO_HPP_
