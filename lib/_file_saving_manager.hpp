#include <vector>
#include <boost/filesystem.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include "_data_info.hpp"
#include "_allocator.hpp"
#include "_block_rw.hpp"
#include "_node.hpp"

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

template <typename _ElementType, size_t T>
class _FileSavingManager{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _FileSavingManager(const std::string &destination,
                     const std::shared_ptr<_DataInfo> &data_info_ptr);

  _FileSavingManager(const std::string &destination,
                     _Node<_ElementType, T> &in_memory_node,
                     const std::shared_ptr<_DataInfo> &data_info_ptr);

  void SetNode(file_pos_t pos, const _Node<_ElementType, T> &node_to_set);

  _Node<_ElementType, T> GetNode(file_pos_t pos) const;

  file_pos_t NewNode();

  file_pos_t NewNode(const _Node<_ElementType, T> &node);

  void DeleteNode(file_pos_t pos);

  void RenameMappedFile(const std::string &new_name);

  ~_FileSavingManager();

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs declaration                                           //
  //////////////////////////////////////////////////////////////////////////////

  struct _NodeParams{
    size_t _info_size;
    size_t _pages_cnt;
    size_t _element_size;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  _NodeParams _node_params;
  _Allocator<_ElementType> _allocator;
  _BlockRW _block_rw;

  std::shared_ptr<_DataInfo> _data_info_ptr;

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  size_t _page_size;
  bool _new_file_flag;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template<typename ElementType, size_t _T>
  friend class BTreeList;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_FileSavingManager<_ElementType, T>::_FileSavingManager(
    const std::string &destination,
    const std::shared_ptr<_DataInfo> &data_info_ptr
) : _page_size(boost::interprocess::mapped_region::get_page_size()),
    _data_info_ptr(data_info_ptr),
    _file_params_ptr(
        std::make_shared<boost::iostreams::mapped_file_params>(destination)) {
  _node_params._info_size = sizeof(struct _Node<_ElementType, T>::_NodeInfo);
  _node_params._element_size = sizeof(_ElementType);
  _node_params._pages_cnt =
      ceil_div(_Node<_ElementType, T>::inmemory_size, _page_size);
  // Prepare opening
  _new_file_flag = !boost::filesystem::exists(_file_params_ptr->path);
  if (_new_file_flag) {
    _file_params_ptr->new_file_size =
        _Allocator<_ElementType>::data_info_size +
        _page_size * _node_params._pages_cnt;  // _DataInfo + one node
  } else {
    _file_params_ptr->new_file_size = 0;
  }
  _file_params_ptr->flags = boost::iostreams::mapped_file::mapmode::readwrite;
  _file_params_ptr->offset = 0;
  // Opening file

  _mapped_file_ptr =
      std::make_shared<boost::iostreams::mapped_file>(*_file_params_ptr);
  _block_rw = _BlockRW(_mapped_file_ptr,
                       _Allocator<_ElementType>::data_info_size,
                       _page_size * _node_params._pages_cnt);
  _allocator = _Allocator<_ElementType>(_mapped_file_ptr,
                                        _file_params_ptr,
                                        _data_info_ptr,
                                        _node_params._pages_cnt * _page_size,
                                        _new_file_flag);
}

template <typename _ElementType, size_t T>
_FileSavingManager<_ElementType, T>::_FileSavingManager(
    const std::string &destination,
    _Node<_ElementType, T> &in_memory_node,
    const std::shared_ptr<_DataInfo> &data_info_ptr
) : _FileSavingManager(destination, data_info_ptr) {
  if (_new_file_flag) {
    in_memory_node = _Node<_ElementType, T>(
        std::vector<_ElementType>{},
        std::vector<file_pos_t>{0},
        std::vector<size_t>{0},
        _Node<_ElementType, T>::_Flags::ROOT |
        _Node<_ElementType, T>::_Flags::LEAF |
        _Node<_ElementType, T>::_Flags::RIGHTEST
    );
    _data_info_ptr->_root_pos = NewNode(in_memory_node);
    _data_info_ptr->_in_memory_node_pos = _data_info_ptr->_root_pos;
  } else {
    in_memory_node = GetNode(_data_info_ptr->_in_memory_node_pos);
  }
}

template <typename _ElementType, size_t T>
void _FileSavingManager<_ElementType, T>::SetNode(
    file_pos_t pos,
    const _Node<_ElementType, T>& node_to_set
) {
  *_block_rw.GetInfoPtr<_ElementType, T>(pos) = node_to_set.GetNodeInfo();

  for (unsigned i = 0; i < node_to_set.Size(); ++i) {
    *_block_rw.GetNodeElementPtr<_ElementType, T>(pos, i) =
        node_to_set._elements[i];
  }
  for (unsigned i = 0; i < node_to_set.Size() + 1; ++i) {
    *_block_rw.GetNodeLinkPtr<_ElementType, T>(pos, i) =
        node_to_set._links[i];
  }
  for (unsigned i = 0; i < node_to_set.Size() + 1; ++i) {
    *_block_rw.GetNodeCCPtr<_ElementType, T>(pos, i) =
        node_to_set._children_cnts[i];
  }
}

template <typename _ElementType, size_t T>
_Node<_ElementType, T> _FileSavingManager<_ElementType, T>::GetNode(
    file_pos_t pos
) const {
  _Node<_ElementType, T> taken_node;
  struct _Node<_ElementType, T>::_NodeInfo taken_info =
      *_block_rw.GetInfoPtr<_ElementType, T>(pos);
  taken_node.Resize(taken_info._elements_cnt);
  taken_node._flags = taken_info._flags;

  for (unsigned i = 0; i < taken_node.Size(); ++i) {
    taken_node.Element(i) =
        *_block_rw.GetNodeElementPtr<_ElementType, T>(pos, i);
  }
  for (unsigned i = 0; i < taken_node.Size() + 1; ++i) {
    taken_node.LinkBefore(i) =
        *_block_rw.GetNodeLinkPtr<_ElementType, T>(pos, i);
  }
  for (unsigned i = 0; i < taken_node.Size() + 1; ++i) {
    taken_node.ChildrenCntBefore(i) =
        *_block_rw.GetNodeCCPtr<_ElementType, T>(pos, i);
  }
  return taken_node;
}

template <typename _ElementType, size_t T>
file_pos_t _FileSavingManager<_ElementType, T>::NewNode() {
  return _allocator.NewNode();
}

template <typename _ElementType, size_t T>
file_pos_t _FileSavingManager<_ElementType, T>::NewNode(
    const _Node<_ElementType, T> &node
) {
  file_pos_t pos = NewNode();
  SetNode(pos, node);
  return pos;
}

template <typename _ElementType, size_t T>
void _FileSavingManager<_ElementType, T>::DeleteNode(file_pos_t pos) {
  _allocator.DeleteNode(pos);
}

template <typename _ElementType, size_t T>
void _FileSavingManager<_ElementType, T>::RenameMappedFile(
    const std::string &new_name
) {
  _mapped_file_ptr->close();
  boost::filesystem::rename(_file_params_ptr->path, new_name);
  _file_params_ptr->path = new_name;
  _file_params_ptr->new_file_size = 0;
  _file_params_ptr->length = _allocator._file_size;
  _mapped_file_ptr->open(*_file_params_ptr);
}

template <typename _ElementType, size_t T>
_FileSavingManager<_ElementType, T>::~_FileSavingManager() {
  *(reinterpret_cast<_DataInfo*>(_mapped_file_ptr->data())) = *_data_info_ptr;
}

#endif //B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
