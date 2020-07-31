//
// Created by gogagum on 16.07.2020.
//

#include "_block_rw.hpp"

#ifndef B_TREE_LIST_LIB__ALLOCATOR_HPP_
#define B_TREE_LIST_LIB__ALLOCATOR_HPP_

#define DEBUG

template <typename __ElementType>
class _Allocator{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _Allocator();

  _Allocator(
      const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
      const std::shared_ptr<boost::iostreams::mapped_file_params> &file_params_ptr,
      size_t block_size,
      bool new_file_flag);

  unsigned int NewNode();

  void DeleteNode(unsigned int pos);

  void _ChangeMaxNumOfNodes(int pages_to_add);

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs declaration                                           //
  //////////////////////////////////////////////////////////////////////////////

  struct _DataInfo{
    int _stack_head_pos;
    unsigned int _free_tail_start;
    unsigned int _max_blocks_cnt;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  _DataInfo _data_info;
  _BlockRW _block_rw;

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  size_t _block_size;
  size_t _file_size;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename _ElementType>
  friend class _FileSavingManager;

  #ifdef DEBUG
  template <typename ElementType>
  friend class BTreeList;
  #endif

};

template <typename __ElementType>
_Allocator<__ElementType>::_Allocator() {}

template <typename __ElementType>
_Allocator<__ElementType>::_Allocator(
    const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
    const std::shared_ptr<boost::iostreams::mapped_file_params> &file_params_ptr,
    size_t block_size,
    bool new_file_flag
) : _mapped_file_ptr(mapped_file_ptr),
    _file_params_ptr(file_params_ptr),
    _block_size(block_size),
    _block_rw(mapped_file_ptr, sizeof(_DataInfo), block_size)
{
  if (!new_file_flag) {
    _data_info = *(reinterpret_cast<_DataInfo*>(_mapped_file_ptr->data()));
  } else { // file is new
    _data_info._free_tail_start = 1;
    _data_info._stack_head_pos = -1;
    _data_info._max_blocks_cnt = 1;
    *(reinterpret_cast<_DataInfo*>(_mapped_file_ptr->data())) = _data_info;
  }
  _file_size = _mapped_file_ptr->size();
}

template <typename __ElementType>
unsigned int _Allocator<__ElementType>::NewNode() {
  unsigned int index_to_return;
  if (_data_info._stack_head_pos != -1) {
    index_to_return = _data_info._stack_head_pos;
    _data_info._stack_head_pos =
        *(_block_rw.GetBlockPtr<unsigned int>(_data_info._stack_head_pos));
  } else {
    index_to_return = _data_info._free_tail_start;
    ++_data_info._free_tail_start;
    if (_data_info._free_tail_start >= _data_info._max_blocks_cnt) {
      this->_ChangeMaxNumOfNodes(5);
    }
  }
  return index_to_return;
}

template <typename __ElementType>
void _Allocator<__ElementType>::DeleteNode(unsigned int pos) {
  if (pos == _data_info._free_tail_start - 1) {
    --_data_info._free_tail_start;
  } else {
    _block_rw.WriteBlock(pos, _data_info._stack_head_pos);
    _data_info._stack_head_pos = pos;
  }
}

template <typename __ElementType>
void _Allocator<__ElementType>::_ChangeMaxNumOfNodes(int blocks_to_add) {
  _mapped_file_ptr->close();
  _file_size += blocks_to_add * _block_size;
  boost::filesystem::resize_file(_file_params_ptr->path, _file_size);
  _file_params_ptr->length = _file_size;
  _file_params_ptr->new_file_size = 0;
  _mapped_file_ptr->open(*_file_params_ptr);
  _data_info._max_blocks_cnt += blocks_to_add;
}


#endif //B_TREE_LIST_LIB__ALLOCATOR_HPP_
