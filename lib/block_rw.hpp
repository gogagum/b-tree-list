//
// Created by gogagum on 16.07.2020.
//

#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#ifndef B_TREE_LIST_LIB__BLOCK_RW_HPP_
#define B_TREE_LIST_LIB__BLOCK_RW_HPP_

typedef uint64_t file_pos_t;
typedef int64_t signed_file_pos_t;

class BlockRW {
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  BlockRW();

  BlockRW(
      const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
      size_t file_info_size,
      size_t block_size);

  template<typename TypeToRead>
  TypeToRead* GetBlockPtr(file_pos_t pos) const;

  template <typename ElementType, size_t T>
  struct Node<ElementType, T>::_NodeInfo* GetInfoPtr(file_pos_t pos) const;

  template<typename ElementType, size_t T>
  ElementType* GetNodeElementPtr(file_pos_t pos, unsigned index) const;

  template<typename ElementType, size_t T>
  file_pos_t* GetNodeLinkPtr(file_pos_t pos, unsigned index) const;

  template<typename ElementType, size_t T>
  size_t* GetNodeCCPtr(file_pos_t pos, unsigned index) const;

  template<typename TypeToWrite>
  void WriteBlock(file_pos_t pos, const TypeToWrite& element);

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  size_t _block_size;  // The same as in allocator which uses this class.

  size_t _file_info_size;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename ElementType>
  friend class Allocator;

  template <typename ElementType, size_t T>
  friend class FileSavingManager;
};

BlockRW::BlockRW() {};

BlockRW::BlockRW(
    const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
    size_t file_info_size,
    size_t block_size)
  : _mapped_file_ptr(mapped_file_ptr),
    _file_info_size(file_info_size),
    _block_size(block_size) {}

template<typename TypeToRead>
TypeToRead* BlockRW::GetBlockPtr(file_pos_t pos) const {
  return reinterpret_cast<TypeToRead*>(_mapped_file_ptr->data() +
      _file_info_size + pos * _block_size);
}

template<typename ElementType, size_t T>
struct Node<ElementType, T>::_NodeInfo* BlockRW::GetInfoPtr(
    file_pos_t pos
) const {
  return reinterpret_cast<struct Node<ElementType, T>::_NodeInfo*>(
      GetBlockPtr<struct Node<ElementType, T>::_NodeInfo>(pos)
  );
};

template<typename ElementType, size_t T>
ElementType* BlockRW::GetNodeElementPtr(file_pos_t pos, unsigned index) const {
  return reinterpret_cast<ElementType*>(
      GetBlockPtr<char>(pos) + Node<ElementType, T>::elements_offset +
      sizeof(ElementType) * index
  );
}

template<typename ElementType, size_t T>
file_pos_t* BlockRW::GetNodeLinkPtr(file_pos_t pos, unsigned index) const {
  return reinterpret_cast<file_pos_t*>(
      GetBlockPtr<char>(pos) + Node<ElementType, T>::links_offset +
      sizeof(file_pos_t) * index
  );
}

template<typename ElementType, size_t T>
size_t* BlockRW::GetNodeCCPtr(file_pos_t pos, unsigned index) const {
  return reinterpret_cast<size_t*>(
      GetBlockPtr<char>(pos) + Node<ElementType, T>::cc_offset +
      sizeof(size_t) * index
  );
}

template<typename TypeToWrite>
void BlockRW::WriteBlock(file_pos_t pos, const TypeToWrite &element) {
  *(_mapped_file_ptr->data() + _file_info_size + pos * _block_size) =
      element;
}

#endif //B_TREE_LIST_LIB__BLOCK_RW_HPP_
