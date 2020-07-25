#include "_allocator.hpp"
#include "_block_rw.hpp"

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

template <typename _ElementType>
class _FileSavingManager{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _FileSavingManager(const std::string &destination, size_t T);

  void SetNode(unsigned int pos, const _Node<_ElementType>& node_to_set);

  _Node<_ElementType> GetNode(unsigned int pos) const;

  unsigned int NewNode();

  void DeleteNode(unsigned int pos);

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

  std::shared_ptr<void*> _mapped_file_ptr;
  int _fd;
  std::string _destination;
  size_t _file_size;
  size_t _page_size;
  size_t _nodes_cnt;
  size_t _max_nodes_cnt;
  size_t _T_parameter;

  //////////////////////////////////////////////////////////////////////////////
  // "Private" methods                                                        //
  //////////////////////////////////////////////////////////////////////////////

  void _AddFileMem(unsigned int pages_to_add);

  void _Reduce(unsigned  int pages_to_reduce);

  void _SetUpFileInfo();

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template<typename ElementType>
  friend class BTreeList;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_FileSavingManager<_ElementType>::_FileSavingManager(
    const std::string &destination,
    size_t T
) {
  _destination = destination;
  _T_parameter = T;
  _node_params._info_size = sizeof(struct _Node<_ElementType>::_NodeInfo);
  _page_size = 1024; // TODO: get this info while running
  _node_params._element_size = sizeof(_ElementType);
  _node_params._pages_cnt =
      ceil_div(_node_params._info_size +
      (_T_parameter * 2 - 1) * _node_params._element_size +
      _T_parameter * 2 * 8 /*links and children cnts*/, _page_size);
  _fd = open(_destination.c_str(), O_CREAT | O_RDWR);
  _block_rw = _BlockRW(_mapped_file_ptr, sizeof(struct _Allocator<_ElementType>::_DataInfo), _page_size * _node_params._pages_cnt);
  _allocator = _Allocator<_ElementType>(_mapped_file_ptr, _fd, _node_params._pages_cnt * _page_size);
}

template <typename _ElementType>
void _FileSavingManager<_ElementType>::_AddFileMem(unsigned int blocks_to_add) {
  munmap(*_mapped_file_ptr, _file_size);
  _file_size += blocks_to_add * _page_size * _node_params._pages_cnt;
  ftruncate(_fd, _file_size);
  mmap(*_mapped_file_ptr, _file_size, PROT_READ | PROT_WRITE, MAP_FILE, _fd, 0);
  _max_nodes_cnt += blocks_to_add;
}

template <typename _ElementType>
void _FileSavingManager<_ElementType>::_Reduce(
    unsigned int blocks_to_reduce
) {
  munmap(*_mapped_file_ptr, _file_size);
  _file_size -= blocks_to_reduce * _page_size * _node_params._pages_cnt;
  ftruncate(_fd, _file_size);
  mmap(*_mapped_file_ptr, _file_size, PROT_READ | PROT_WRITE, MAP_FILE, _fd, 0);
  _max_nodes_cnt -= blocks_to_reduce;
}

template <typename _ElementType>
void _FileSavingManager<_ElementType>::SetNode(
    unsigned int pos,
    const _Node<_ElementType>& node_to_set
) {
  void* ptr = _block_rw.GetBlockPtr<void>(pos);
  void* elements_starting_pos =
      (char*)ptr + sizeof(struct _Node<_ElementType>::_NodeInfo);
  void* links_starting_pos =
      (char*)elements_starting_pos +
      2 * _T_parameter * _node_params._element_size;
  void* children_cnts_starting_pos =
      (char*)links_starting_pos + (2 * _T_parameter + 1) * 4 /*links*/;

  *(static_cast<typename _Node<_ElementType>::_NodeInfo*>(ptr)) = node_to_set._info;

  for (unsigned int i = 0; i < node_to_set._elements.size(); ++i) {
    *(static_cast<_ElementType*>(elements_starting_pos) + i) =
        node_to_set._elements[i];
  }
  for (unsigned int i = 0; i < node_to_set._links.size(); ++i) {
    *(static_cast<uint32_t*>(links_starting_pos) + i) =
        node_to_set._links[i];
  }
  for (unsigned int i = 0; i < node_to_set._children_cnts.size(); ++i) {
    *(static_cast<uint32_t*>(children_cnts_starting_pos) + i) =
        node_to_set._children_cnts[i];
  }
}

template <typename _ElementType>
_Node<_ElementType> _FileSavingManager<_ElementType>::GetNode(
    unsigned int pos
) const {
  _Node<_ElementType> taken_node;
  void* ptr = _block_rw.GetBlockPtr<void>(pos);
  void* elements_starting_position =
      (char*)ptr + sizeof(_Node<_ElementType>::_NodeInfo);
  void* links_starting_pos =
      (char*)elements_starting_position +
      2 * _T_parameter * _node_params._element_size;
  void* children_cnts_starting_pos =
      (char*)links_starting_pos + (2 * _T_parameter + 1) * 4 /*links*/;

  taken_node._info =
      *(static_cast<typename _Node<_ElementType>::_NodeInfo*>(ptr));

  for (unsigned int i = 0; i < taken_node._info._elements_cnt; ++i) {
    taken_node._elements.push_back(
        *(static_cast<_ElementType*>(elements_starting_position) + i));
  }
  for (unsigned int i = 0; i < taken_node._info._elements_cnt + 1; ++i) {
    taken_node._links.push_back(
        *(static_cast<uint32_t *>(links_starting_pos) + i));
  }
  for (unsigned int i = 0; i < taken_node._cildren_cnts.size(); ++i) {
    taken_node._children_cnts.push_back(
        *(static_cast<uint32_t*>(children_cnts_starting_pos) + i));
  }
  return taken_node;
}

template <typename _ElementType>
unsigned int _FileSavingManager<_ElementType>::NewNode() {
  return _allocator.NewNode();
}

template <typename _ElementType>
void _FileSavingManager<_ElementType>::DeleteNode(unsigned int pos) {

}

template <typename _ElementType>
_FileSavingManager<_ElementType>::~_FileSavingManager() {
  if (!munmap(*_mapped_file_ptr, _file_size)) {
    // TODO: Error
  };
}


#endif //B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
