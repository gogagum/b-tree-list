//
// Created by gogagum on 16.07.2020.
//

#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include "data_info.hpp"

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
      size_t first_node_offset,
      size_t block_size);

  DataInfo* GetDataInfoPtr();

  const DataInfo* GetDataInfoPtr() const;

  template<typename TypeToRead>
  TypeToRead* GetBlockPtr(file_pos_t pos);

  template<typename TypeToRead>
  const TypeToRead* GetBlockPtr(file_pos_t pos) const;

  template <typename ElementType, size_t T>
  struct Node<ElementType, T>::_NodeInfo* GetNodeInfoPtr(file_pos_t pos);

  template <typename ElementType, size_t T>
  const struct Node<ElementType, T>::_NodeInfo*
      GetNodeInfoPtr(file_pos_t pos) const;

  template<typename ElementType, size_t T>
  char* GetNodeElementsBegPtr(file_pos_t pos);

  template<typename ElementType, size_t T>
  const char* GetNodeElementsBegPtr(file_pos_t pos) const;

  template<typename ElementType, size_t T>
  ElementType* GetNodeElementPtr(file_pos_t pos, unsigned index);

  template<typename ElementType, size_t T>
  char* GetNodeLinksBegPtr(file_pos_t pos);

  template<typename ElementType, size_t T>
  const char* GetNodeLinksBegPtr(file_pos_t pos) const;

  template<typename ElementType, size_t T>
  [[maybe_unused]] file_pos_t* GetNodeLinkPtr(file_pos_t pos, unsigned index);

  template<typename ElementType, size_t T>
  char* GetNodeCCBegPtr(file_pos_t pos);

  template<typename ElementType, size_t T>
  const char* GetNodeCCBegPtr(file_pos_t pos) const;

  template<typename ElementType, size_t T>
  size_t* GetNodeCCPtr(file_pos_t pos, unsigned index);

  template<typename TypeToWrite>
  void WriteBlock(file_pos_t pos, const TypeToWrite& element);

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  size_t _block_size;  // The same as in allocator which uses this class.

  size_t _first_node_offset;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename ElementType>
  friend class Allocator;

  template <typename ElementType, size_t T>
  friend class FileSavingManager;

  template <typename ElementType, size_t T>
  friend class BTreeList;
};

BlockRW::BlockRW() {};

BlockRW::BlockRW(
    const std::shared_ptr<boost::iostreams::mapped_file> &mapped_file_ptr,
    size_t first_node_offset,
    size_t block_size)
  : _mapped_file_ptr(mapped_file_ptr),
    _first_node_offset(first_node_offset),
    _block_size(block_size) {}

DataInfo* BlockRW::GetDataInfoPtr() {
  return reinterpret_cast<DataInfo*>(_mapped_file_ptr->data());
}


const DataInfo* BlockRW::GetDataInfoPtr() const {
  return reinterpret_cast<const DataInfo*>(_mapped_file_ptr->data());
}

template<typename TypeToRead>
TypeToRead* BlockRW::GetBlockPtr(file_pos_t pos) {
  return reinterpret_cast<TypeToRead*>(_mapped_file_ptr->data() +
      _first_node_offset + pos * _block_size);
}

template<typename TypeToRead>
const TypeToRead* BlockRW::GetBlockPtr(file_pos_t pos) const {
  return reinterpret_cast<TypeToRead*>(_mapped_file_ptr->data() +
      _first_node_offset + pos * _block_size);
}

template<typename ElementType, size_t T>
struct Node<ElementType, T>::_NodeInfo* BlockRW::GetNodeInfoPtr(
    file_pos_t pos
) {
  return reinterpret_cast<struct Node<ElementType, T>::_NodeInfo*>(
      GetBlockPtr<struct Node<ElementType, T>::_NodeInfo>(pos)
  );
};

template<typename ElementType, size_t T>
const struct Node<ElementType, T>::_NodeInfo* BlockRW::GetNodeInfoPtr(
    file_pos_t pos
) const {
  return reinterpret_cast<const struct Node<ElementType, T>::_NodeInfo*>(
      GetBlockPtr<struct Node<ElementType, T>::_NodeInfo>(pos)
  );
};

template<typename ElementType, size_t T>
char* BlockRW::GetNodeElementsBegPtr(file_pos_t pos) {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::elements_offset;
}

template<typename ElementType, size_t T>
const char* BlockRW::GetNodeElementsBegPtr(file_pos_t pos) const {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::elements_offset;
}

template<typename ElementType, size_t T>
ElementType* BlockRW::GetNodeElementPtr(file_pos_t pos, unsigned index) {
  return reinterpret_cast<ElementType*>(
      GetNodeElementsBegPtr<ElementType, T>(pos) +  sizeof(ElementType) * index
  );
}

template<typename ElementType, size_t T>
[[maybe_unused]] file_pos_t* BlockRW::GetNodeLinkPtr(file_pos_t pos,
                                                     unsigned index) {
  return reinterpret_cast<file_pos_t*>(
      GetBlockPtr<char>(pos) + Node<ElementType, T>::links_offset +
      sizeof(file_pos_t) * index
  );
}

template<typename ElementType, size_t T>
char* BlockRW::GetNodeCCBegPtr(file_pos_t pos) {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::cc_offset;
}

template<typename ElementType, size_t T>
const char* BlockRW::GetNodeCCBegPtr(file_pos_t pos) const {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::cc_offset;
}

template<typename ElementType, size_t T>
size_t* BlockRW::GetNodeCCPtr(file_pos_t pos, unsigned index) {
  return reinterpret_cast<size_t*>(
      GetNodeCCBegPtr<ElementType, T>(pos) + sizeof(size_t) * index
  );
}

template<typename ElementType, size_t T>
char* BlockRW::GetNodeLinksBegPtr(file_pos_t pos) {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::links_offset;
}

template<typename ElementType, size_t T>
const char* BlockRW::GetNodeLinksBegPtr(file_pos_t pos) const {
  return GetBlockPtr<char>(pos) + Node<ElementType, T>::links_offset;
}

template<typename TypeToWrite>
void BlockRW::WriteBlock(file_pos_t pos, const TypeToWrite &element) {
  *(_mapped_file_ptr->data() + _first_node_offset + pos * _block_size) =
      element;
}

#endif //B_TREE_LIST_LIB__BLOCK_RW_HPP_
