#include <vector>
#include <boost/filesystem.hpp>
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

////////////////////////////////////////////////////////////////////////////////
// File saving manager                                                        //
////////////////////////////////////////////////////////////////////////////////

inline int ceil_div(int a, int b) {
  return (a - 1) / b + 1;
}

template <typename ElementType, size_t T>
class FileSavingManager{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  FileSavingManager(const std::string &destination,
                    const std::shared_ptr<DataInfo> &data_info_ptr);

  FileSavingManager(const std::string &destination,
                    Node<ElementType, T> &in_memory_node,
                    const std::shared_ptr<DataInfo> &data_info_ptr);

  void SetNode(file_pos_t pos, const Node<ElementType, T> &node_to_set);

  Node<ElementType, T> GetNode(file_pos_t pos) const;

  file_pos_t NewNode();

  file_pos_t NewNode(const Node<ElementType, T> &node);

  void DeleteNode(file_pos_t pos);

  void RenameMappedFile(const std::string &new_name);

  ~FileSavingManager();

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs declaration                                           //
  //////////////////////////////////////////////////////////////////////////////

  struct _NodeParams{
    size_t _info_size;
    size_t _pages_cnt;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  _NodeParams _node_params;
  Allocator<ElementType> _allocator;
  BlockRW _block_rw;

  std::shared_ptr<DataInfo> _data_info_ptr;

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  size_t _page_size;
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
    const std::shared_ptr<DataInfo> &data_info_ptr
) : _page_size(boost::interprocess::mapped_region::get_page_size()),
    _data_info_ptr(data_info_ptr),
    _file_params_ptr(
        std::make_shared<boost::iostreams::mapped_file_params>(destination)) {
  _node_params._info_size = sizeof(struct Node<ElementType, T>::_NodeInfo);
  _node_params._pages_cnt =
      ceil_div(Node<ElementType, T>::inmemory_size, _page_size);
  // Prepare opening
  _new_file_flag = !boost::filesystem::exists(_file_params_ptr->path);
  if (_new_file_flag) {
    _file_params_ptr->new_file_size =
        Allocator<ElementType>::data_info_size +
        _page_size * _node_params._pages_cnt;  // DataInfo + one node
  } else {
    _file_params_ptr->new_file_size = 0;
  }
  _file_params_ptr->flags = boost::iostreams::mapped_file::mapmode::readwrite;
  _file_params_ptr->offset = 0;
  // Opening file

  _mapped_file_ptr =
      std::make_shared<boost::iostreams::mapped_file>(*_file_params_ptr);
  _block_rw = BlockRW(_mapped_file_ptr,
                      Allocator<ElementType>::data_info_size,
                      _page_size * _node_params._pages_cnt);
  _allocator = Allocator<ElementType>(_mapped_file_ptr,
                                       _file_params_ptr,
                                       _data_info_ptr,
                                       _node_params._pages_cnt * _page_size,
                                       _new_file_flag);
}

template <typename ElementType, size_t T>
FileSavingManager<ElementType, T>::FileSavingManager(
    const std::string &destination,
    Node<ElementType, T> &in_memory_node,
    const std::shared_ptr<DataInfo> &data_info_ptr
) : FileSavingManager(destination, data_info_ptr) {
  if (_new_file_flag) {
    in_memory_node = Node<ElementType, T>(
        std::vector<ElementType>{},
        std::vector<file_pos_t>{0},
        std::vector<size_t>{0},
        Node<ElementType, T>::_Flags::ROOT |
        Node<ElementType, T>::_Flags::LEAF
    );
    _data_info_ptr->_root_pos = NewNode(in_memory_node);
    _data_info_ptr->_in_memory_node_pos = _data_info_ptr->_root_pos;
  } else {
    in_memory_node = GetNode(_data_info_ptr->_in_memory_node_pos);
  }
}

template <typename ElementType, size_t T>
void FileSavingManager<ElementType, T>::SetNode(
    file_pos_t pos,
    const Node<ElementType, T>& node_to_set
) {
  *_block_rw.GetInfoPtr<ElementType, T>(pos) = node_to_set.GetNodeInfo();

  for (unsigned i = 0; i < node_to_set.Size(); ++i) {
    *_block_rw.GetNodeElementPtr<ElementType, T>(pos, i) =
        node_to_set._elements[i];
  }
  for (unsigned i = 0; i < node_to_set.Size() + 1; ++i) {
    *_block_rw.GetNodeLinkPtr<ElementType, T>(pos, i) =
        node_to_set._links[i];
  }
  for (unsigned i = 0; i < node_to_set.Size() + 1; ++i) {
    *_block_rw.GetNodeCCPtr<ElementType, T>(pos, i) =
        node_to_set._children_cnts[i];
  }
}

template <typename ElementType, size_t T>
Node<ElementType, T> FileSavingManager<ElementType, T>::GetNode(
    file_pos_t pos
) const {
  Node<ElementType, T> taken_node;
  struct Node<ElementType, T>::_NodeInfo taken_info =
      *_block_rw.GetInfoPtr<ElementType, T>(pos);
  taken_node.Resize(taken_info._elements_cnt);
  taken_node._flags = taken_info._flags;

  for (unsigned i = 0; i < taken_node.Size(); ++i) {
    taken_node.Element(i) =
        *_block_rw.GetNodeElementPtr<ElementType, T>(pos, i);
  }
  for (unsigned i = 0; i < taken_node.Size() + 1; ++i) {
    taken_node.LinkBefore(i) =
        *_block_rw.GetNodeLinkPtr<ElementType, T>(pos, i);
  }
  for (unsigned i = 0; i < taken_node.Size() + 1; ++i) {
    taken_node.ChildrenCntBefore(i) =
        *_block_rw.GetNodeCCPtr<ElementType, T>(pos, i);
  }
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
  boost::filesystem::rename(_file_params_ptr->path, new_name);
  _file_params_ptr->path = new_name;
  _file_params_ptr->new_file_size = 0;
  _file_params_ptr->length = _allocator._file_size;
  _mapped_file_ptr->open(*_file_params_ptr);
}

template <typename ElementType, size_t T>
FileSavingManager<ElementType, T>::~FileSavingManager() {
  *(reinterpret_cast<DataInfo*>(_mapped_file_ptr->data())) = *_data_info_ptr;
}

#endif //B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
