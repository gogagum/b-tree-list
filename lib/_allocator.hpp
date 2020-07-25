//
// Created by gogagum on 16.07.2020.
//

#include "_block_rw.hpp"


#ifndef B_TREE_LIST_LIB__ALLOCATOR_HPP_
#define B_TREE_LIST_LIB__ALLOCATOR_HPP_

template <typename __ElementType>
class _Allocator{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _Allocator();

  _Allocator(std::shared_ptr<void*> mapped_file_ptr,
             int fd,
             size_t block_size);

  unsigned int NewNode();

  void DeleteNode(unsigned int pos);

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs declaration                                           //
  //////////////////////////////////////////////////////////////////////////////

  struct _DataInfo{
    int _stack_head_pos;
    unsigned int _free_tail_start;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  _DataInfo _data_info;
  _BlockRW _block_rw;

  std::shared_ptr<void*> _mapped_file_ptr;
  size_t _block_size;
  size_t _file_size;
  unsigned int _root_link;

  size_t _max_block_index;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename _ElementType>
  friend class _FileSavingManager;
};

template <typename __ElementType>
_Allocator<__ElementType>::_Allocator() {}

template <typename __ElementType>
_Allocator<__ElementType>::_Allocator(
    std::shared_ptr<void*> mapped_file_ptr,
    int fd,
    size_t block_size
) : _mapped_file_ptr(mapped_file_ptr),
    _block_size(),
    _block_rw(_mapped_file_ptr, sizeof(_DataInfo), block_size)
{
  struct stat st;
  fstat(fd, &st);
  _file_size = st.st_size;
  if (_file_size != 0) {
    mmap(*_mapped_file_ptr, _file_size, PROT_READ | PROT_WRITE, MAP_FILE, fd, 0);
    _data_info = *(static_cast<_DataInfo*>(*_mapped_file_ptr));
  } else {
    unsigned int _info_size = sizeof(_DataInfo);
    _data_info._free_tail_start = 1;
    _data_info._stack_head_pos = -1;
    _root_link = _info_size;
    _file_size = _info_size + _block_size;
    ftruncate(fd, _file_size);
    mmap(*_mapped_file_ptr, _file_size, PROT_READ | PROT_WRITE, MAP_FILE, fd, 0);
    *(static_cast<_DataInfo*>(*_mapped_file_ptr)) = _data_info;
  }
}

template <typename __ElementType>
unsigned int _Allocator<__ElementType>::NewNode() {
  int index_to_return = 0;
  if (_data_info._stack_head_pos != -1) {
    index_to_return = _data_info._stack_head_pos;
    _data_info._stack_head_pos =
        *(_block_rw.GetBlockPtr<unsigned int>(_data_info._stack_head_pos));
  } else if (_data_info._free_tail_start < _max_block_index) {
    index_to_return = _data_info._free_tail_start;
    ++_data_info._free_tail_start;
  } else {  // Need to make file bigger
    this->_AddFileMem(1);
    index_to_return = _data_info._free_tail_start;
    ++_data_info._free_tail_start;
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


#endif //B_TREE_LIST_LIB__ALLOCATOR_HPP_
