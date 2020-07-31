#include <boost/filesystem.hpp>
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

  std::shared_ptr<boost::iostreams::mapped_file> _mapped_file_ptr;
  std::shared_ptr<boost::iostreams::mapped_file_params> _file_params_ptr;
  size_t _page_size;
  size_t _T_parameter;

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
) : _T_parameter(T),
    _page_size(4096) // TODO: get this info while running
{
  _file_params_ptr =
      std::make_shared<boost::iostreams::mapped_file_params>(destination);
  _node_params._info_size = sizeof(struct _Node<_ElementType>::_NodeInfo);
  _node_params._element_size = sizeof(_ElementType);
  _node_params._pages_cnt =
      ceil_div(_node_params._info_size +
      (_T_parameter * 2 - 1) * _node_params._element_size +
      _T_parameter * 2 * 8 /*links and children cnts*/, _page_size);
  // Prepare opening
  _file_params_ptr->new_file_size =
      sizeof(struct _Allocator<_ElementType>::_DataInfo) +
      _page_size * _node_params._pages_cnt;  // _DataInfo + one node
  _file_params_ptr->flags = boost::iostreams::mapped_file::mapmode::readwrite;
  _file_params_ptr->offset = 0;
  // Opening file
  bool new_file_flag = !boost::filesystem::exists(_file_params_ptr->path);
  _mapped_file_ptr =
      std::make_shared<boost::iostreams::mapped_file>(*_file_params_ptr);
  _block_rw = _BlockRW(_mapped_file_ptr,
                       sizeof(struct _Allocator<_ElementType>::_DataInfo),
                       _page_size * _node_params._pages_cnt);
  _allocator = _Allocator<_ElementType>(_mapped_file_ptr,
                                        _file_params_ptr,
                                        _node_params._pages_cnt * _page_size,
                                        new_file_flag);
}

template <typename _ElementType>
void _FileSavingManager<_ElementType>::SetNode(
    unsigned int pos,
    const _Node<_ElementType>& node_to_set
) {
  char* ptr = _block_rw.GetBlockPtr<char>(pos);
  char* elements_starting_pos =
      ptr + sizeof(struct _Node<_ElementType>::_NodeInfo);
  char* links_starting_pos =
      elements_starting_pos +
      (2 * _T_parameter - 1) * _node_params._element_size;
  char* children_cnts_starting_pos =
      links_starting_pos + (2 * _T_parameter - 1) * 4 /*links*/;

  *(reinterpret_cast<struct _Node<_ElementType>::_NodeInfo*>(ptr)) =
      node_to_set._info;

  for (unsigned int i = 0; i < node_to_set._elements.size(); ++i) {
    *(reinterpret_cast<_ElementType*>(elements_starting_pos) + i) =
        node_to_set._elements[i];
  }
  for (unsigned int i = 0; i < node_to_set._links.size(); ++i) {
    *(reinterpret_cast<uint32_t*>(links_starting_pos) + i) =
        node_to_set._links[i];
  }
  for (unsigned int i = 0; i < node_to_set._children_cnts.size(); ++i) {
    *(reinterpret_cast<uint32_t*>(children_cnts_starting_pos) + i) =
        node_to_set._children_cnts[i];
  }
}

template <typename _ElementType>
_Node<_ElementType> _FileSavingManager<_ElementType>::GetNode(
    unsigned int pos
) const {
  _Node<_ElementType> taken_node;
  char* ptr = _block_rw.GetBlockPtr<char>(pos);
  char* elements_starting_position =
      ptr + sizeof(struct _Node<_ElementType>::_NodeInfo);
  char* links_starting_pos =
      elements_starting_position +
      (2 * _T_parameter - 1) * _node_params._element_size;
  char* children_cnts_starting_pos =
      links_starting_pos + (2 * _T_parameter - 1) * 4 /*links*/;

  taken_node._info =
      *(reinterpret_cast<struct _Node<_ElementType>::_NodeInfo*>(ptr));

  for (unsigned int i = 0; i < taken_node._info._elements_cnt; ++i) {
    taken_node._elements.push_back(
        *(reinterpret_cast<_ElementType*>(elements_starting_position) + i));
  }
  for (unsigned int i = 0; i < taken_node._info._elements_cnt + 1; ++i) {
    taken_node._links.push_back(
        *(reinterpret_cast<uint32_t*>(links_starting_pos) + i));
  }
  for (unsigned int i = 0; i < taken_node._info._elements_cnt + 1; ++i) {
    taken_node._children_cnts.push_back(
        *(reinterpret_cast<uint32_t*>(children_cnts_starting_pos) + i));
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
_FileSavingManager<_ElementType>::~_FileSavingManager() {}


#endif //B_TREE_LIST_LIB__FILE_SAVING_MANAGER_HPP_
