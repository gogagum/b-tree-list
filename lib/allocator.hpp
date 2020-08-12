//
// Created by gogagum on 16.07.2020.
//

#include <boost/iostreams/device/mapped_file.hpp>
#include "data_info.hpp"
#include "block_rw.hpp"

#ifndef B_TREE_LIST_LIB__ALLOCATOR_HPP_
#define B_TREE_LIST_LIB__ALLOCATOR_HPP_

#define DEBUG

typedef uint64_t file_pos_t;
typedef int64_t signed_file_pos_t;

inline int ceil_div(int a, int b) {
  return (a - 1) / b + 1;
}

template <typename ElementType>
class Allocator{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  // Default constructor
  Allocator();

  // Constructor with parameters
  Allocator(
      const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
      const std::shared_ptr<boost::iostreams::mapped_file_params>
          &file_params_ptr,
      const std::shared_ptr<DataInfo> &data_info_ptr,
      size_t block_size,
      bool new_file_flag);

  [[nodiscard]] file_pos_t NewNode();

  void DeleteNode(file_pos_t pos);

  void _ChangeMaxNumOfNodes(int pages_to_add);

  ~Allocator();

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<DataInfo> _data_info_ptr;
  BlockRW _block_rw;

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  size_t _block_size;
  size_t _file_size;

  //////////////////////////////////////////////////////////////////////////////
  // Static fields                                                            //
  //////////////////////////////////////////////////////////////////////////////

  const static size_t data_info_size = sizeof(DataInfo);

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename _ElementType, size_t T>
  friend class FileSavingManager;

  #ifdef DEBUG
  template <typename _ElementType, size_t T>
  friend class BTreeList;
  #endif
};

template <typename ElementType>
Allocator<ElementType>::Allocator() {}

template <typename ElementType>
Allocator<ElementType>::Allocator(
    const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
    const std::shared_ptr<boost::iostreams::mapped_file_params> &file_params_ptr,
    const std::shared_ptr<DataInfo> &data_info_ptr,
    size_t block_size,
    bool new_file_flag
) : _mapped_file_ptr(mapped_file_ptr),
    _file_params_ptr(file_params_ptr),
    _block_size(block_size),
    _block_rw(mapped_file_ptr,
              ceil_div(sizeof(DataInfo), boost::interprocess::mapped_region::get_page_size()),
              block_size),
    _data_info_ptr(data_info_ptr)
{
  if (!new_file_flag) {
    *_data_info_ptr = *(reinterpret_cast<DataInfo*>(_mapped_file_ptr->data()));
  } else { // file is new
    _data_info_ptr->_free_tail_start = 0;
    _data_info_ptr->_stack_head_pos = -1;
    _data_info_ptr->_max_blocks_cnt = 1;
    _data_info_ptr->_root_pos = 0;
    _data_info_ptr->_in_memory_node_pos = 0;
    *(reinterpret_cast<DataInfo*>(_mapped_file_ptr->data())) = *_data_info_ptr;
  }
  _file_size = _mapped_file_ptr->size();
}

template <typename ElementType>
file_pos_t Allocator<ElementType>::NewNode() {
  file_pos_t index_to_return;
  if (_data_info_ptr->_stack_head_pos != -1) {
    index_to_return = static_cast<file_pos_t>(_data_info_ptr->_stack_head_pos);
    _data_info_ptr->_stack_head_pos =
        *(_block_rw.GetBlockPtr<signed_file_pos_t>(_data_info_ptr->_stack_head_pos));
  } else {
    index_to_return = _data_info_ptr->_free_tail_start;
    ++_data_info_ptr->_free_tail_start;
    if (_data_info_ptr->_free_tail_start >= _data_info_ptr->_max_blocks_cnt) {
      this->_ChangeMaxNumOfNodes(100);
    }
  }
  return index_to_return;
}

template <typename ElementType>
void Allocator<ElementType>::DeleteNode(file_pos_t pos) {
  if (pos == _data_info_ptr->_free_tail_start - 1) {
    --_data_info_ptr->_free_tail_start;
  } else {
    _block_rw.WriteBlock(pos, _data_info_ptr->_stack_head_pos);
    _data_info_ptr->_stack_head_pos = pos;
  }
}

template <typename ElementType>
void Allocator<ElementType>::_ChangeMaxNumOfNodes(int blocks_to_add) {
  _mapped_file_ptr->close();
  _file_size += blocks_to_add * _block_size;
  boost::filesystem::resize_file(_file_params_ptr->path, _file_size);
  _file_params_ptr->length = _file_size;
  _file_params_ptr->new_file_size = 0;
  _mapped_file_ptr->open(*_file_params_ptr);
  _data_info_ptr->_max_blocks_cnt += blocks_to_add;
}

template <typename ElementType>
Allocator<ElementType>::~Allocator() = default;

#endif //B_TREE_LIST_LIB__ALLOCATOR_HPP_
