#include <vector>
#include <filesystem>
#include <boost/interprocess/mapped_region.hpp>
#include "data_info.hpp"
#include "allocator.hpp"
#include "block_rw.hpp"
#include "node.hpp"

//
// Created by gogagum on 14.07.2020.
//

#ifndef B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
#define B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_

inline size_t GetPagesSize(size_t inmemory_size) {
  return CeilDiv(inmemory_size,
                 boost::interprocess::mapped_region::get_page_size());
}

////////////////////////////////////////////////////////////////////////////////
// File saving manager                                                        //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
class FileSavingManager{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  // File manager constructor
  FileSavingManager(const std::string &destination,
                    const std::shared_ptr<DataInfo> &data_info_ptr,
                    bool file_creation_expected = false);

  // Set node to the position pos
  void SetNode(file_pos_t pos, const Node<ElementType, T> &node_to_set);

  // Get node from position pos
  Node<ElementType, T> GetNode(file_pos_t pos) const;

  // Add new node to memory and return position
  file_pos_t NewNode();

  // Add new node to memory and set node to this position
  file_pos_t NewNode(const Node<ElementType, T> &node);

  // Delete node (free memory) from pos position in file
  void DeleteNode(file_pos_t pos);

  // Rename mapped file
  void RenameMappedFile(const std::string &new_name);

  ~FileSavingManager();

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  Allocator<ElementType> _allocator;
  BlockRW _block_rw;

  std::shared_ptr<DataInfo> _data_info_ptr;

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  bool _new_file_flag;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template<typename _ElementType, size_t _T>
  friend class BTreeList;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
FileSavingManager<ElementType, T>::FileSavingManager(
    const std::string &destination,
    const std::shared_ptr<DataInfo> &data_info_ptr,
    bool file_creation_expected
) : _data_info_ptr(data_info_ptr),
    _file_params_ptr(
        std::make_shared<boost::iostreams::mapped_file_params>(destination)
    ) {
  // Prepare opening
  size_t page_size = boost::interprocess::mapped_region::get_page_size();
  _new_file_flag = !std::filesystem::exists(_file_params_ptr->path);
  if (file_creation_expected || _new_file_flag) {
    std::filesystem::remove(_file_params_ptr->path);
    _new_file_flag = true;
  }
  if (_new_file_flag) {
    _file_params_ptr->new_file_size =
        Allocator<ElementType>::data_info_size +
        GetPagesSize(Node<ElementType, T>::inmemory_size) * page_size;
  } else {
    _file_params_ptr->new_file_size = 0;
  }
  _file_params_ptr->flags = boost::iostreams::mapped_file::mapmode::readwrite;
  _file_params_ptr->offset = 0;
  // Opening file
  _mapped_file_ptr =
      std::make_shared<boost::iostreams::mapped_file>(*_file_params_ptr);
  _block_rw = BlockRW(
      _mapped_file_ptr,
      GetPagesSize(Allocator<ElementType>::data_info_size) * page_size,
      GetPagesSize(Node<ElementType, T>::inmemory_size) * page_size
  );
  _allocator = Allocator<ElementType>(
      _mapped_file_ptr,
      _file_params_ptr,
      _data_info_ptr,
      GetPagesSize(Node<ElementType, T>::inmemory_size) * page_size,
      _new_file_flag
  );
  if (_new_file_flag) {
    auto root_node = Node<ElementType, T>(
        std::vector<ElementType>{},
        std::vector<file_pos_t>{0},
        std::vector<size_t>{0},
        Node<ElementType, T>::_Flags::ROOT | Node<ElementType, T>::_Flags::LEAF
    );
    _data_info_ptr->_root_pos = NewNode(root_node);
  }
}

template <typename ElementType, size_t T>
void FileSavingManager<ElementType, T>::SetNode(
    file_pos_t pos,
    const Node<ElementType, T>& node_to_set
) {
  *_block_rw.GetNodeInfoPtr<ElementType, T>(pos) = node_to_set.GetNodeInfo();

  std::memcpy(_block_rw.GetNodeElementsBegPtr<ElementType, T>(pos),
              node_to_set._elements.data(), node_to_set.ElementsArraySize());
  std::memcpy(_block_rw.GetNodeLinksBegPtr<ElementType, T>(pos),
              node_to_set._links.data(), node_to_set.LinksArraySize());
  std::memcpy(_block_rw.GetNodeCCBegPtr<ElementType, T>(pos),
              node_to_set._children_cnts.data(), node_to_set.CCArraySize());
}

template <typename ElementType, size_t T>
Node<ElementType, T> FileSavingManager<ElementType, T>::GetNode(
    file_pos_t pos
) const {
  Node<ElementType, T> taken_node;
  struct Node<ElementType, T>::_NodeInfo taken_info =
      *_block_rw.GetNodeInfoPtr<ElementType, T>(pos);
  taken_node.Resize(taken_info._elements_cnt);
  taken_node._flags = taken_info._flags;

  std::memcpy(taken_node._elements.data(),
              _block_rw.GetNodeElementsBegPtr<ElementType, T>(pos),
              taken_node.ElementsArraySize());
  std::memcpy(taken_node._links.data(),
              _block_rw.GetNodeLinksBegPtr<ElementType, T>(pos),
              taken_node.LinksArraySize());
  std::memcpy(taken_node._children_cnts.data(),
              _block_rw.GetNodeCCBegPtr<ElementType, T>(pos),
              taken_node.CCArraySize());
  return taken_node;
}

template <typename ElementType, size_t T>
file_pos_t FileSavingManager<ElementType, T>::NewNode() {
  return _allocator.NewNode();
}

template <typename ElementType, size_t T>
file_pos_t FileSavingManager<ElementType, T>::NewNode(
    const Node<ElementType, T> &node
) {
  file_pos_t pos = NewNode();
  SetNode(pos, node);
  return pos;
}

template <typename ElementType, size_t T>
void FileSavingManager<ElementType, T>::DeleteNode(file_pos_t pos) {
  _allocator.DeleteNode(pos);
}

template <typename ElementType, size_t T>
void FileSavingManager<ElementType, T>::RenameMappedFile(
    const std::string &new_name
) {
  _mapped_file_ptr->close();
  std::filesystem::rename(_file_params_ptr->path, new_name);
  _file_params_ptr->path = new_name;
  _file_params_ptr->new_file_size = 0;
  _file_params_ptr->length = _allocator._file_size;
  _mapped_file_ptr->open(*_file_params_ptr);
}

template <typename ElementType, size_t T>
FileSavingManager<ElementType, T>::~FileSavingManager() {
  *_block_rw.GetDataInfoPtr() = *_data_info_ptr;
}

#endif //B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
