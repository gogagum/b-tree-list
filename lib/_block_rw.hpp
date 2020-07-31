//
// Created by gogagum on 16.07.2020.
//

#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#ifndef B_TREE_LIST_LIB__BLOCK_RW_HPP_
#define B_TREE_LIST_LIB__BLOCK_RW_HPP_

class _BlockRW {
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _BlockRW();

  _BlockRW(
      const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
      size_t file_info_size,
      size_t block_size);

  template<typename TypeToRead>
  TypeToRead* GetBlockPtr(unsigned int pos) const;

  template<typename TypeToWrite>
  void WriteBlock(unsigned int pos, const TypeToWrite& element);

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  size_t _block_size;  // The same as in allocator which uses this class.

  size_t _file_info_size;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename __ElementType>
  friend class _Allocator;

  template <typename _ElementType>
  friend class _FileSavingManager;
};

_BlockRW::_BlockRW() {};

_BlockRW::_BlockRW(
    const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
    size_t file_info_size,
    size_t block_size)
  : _mapped_file_ptr(mapped_file_ptr),
    _file_info_size(file_info_size),
    _block_size(block_size) {}

template<typename TypeToRead>
TypeToRead* _BlockRW::GetBlockPtr(unsigned int pos) const {
  return reinterpret_cast<TypeToRead*>(_mapped_file_ptr->data() + _file_info_size +
         pos * _block_size);
}

template<typename TypeToWrite>
void _BlockRW::WriteBlock(unsigned int pos, const TypeToWrite &element) {
  *(_mapped_file_ptr->data() + _file_info_size + pos * _block_size) =
      element;
}

#endif //B_TREE_LIST_LIB__BLOCK_RW_HPP_
