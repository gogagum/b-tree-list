#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "_node.hpp"
#include "_file_saving_manager.hpp"

#ifndef B_TREE_LIST_LIBRARY_H
#define B_TREE_LIST_LIBRARY_H

////////////////////////////////////////////////////////////////////////////////
// Subsidiary classes                                                         //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
class BTreeList{
 public:

  BTreeList(size_t size = 0, size_t limit = 256, size_t T_parameter);

  void Insert(const unsigned int index, const ElementType& e);

  ElementType Extract(const unsigned int index);

  void Set(const unsigned int) const;

  ElementType Get(const unsigned int index);

  size_t Size() const;

 private:
  //////////////////////////////////////////////////////////////////////////////
  // Private fields                                                           //
  //////////////////////////////////////////////////////////////////////////////

  unsigned int _root_link;

  _FileSavingManager<ElementType> _file_manager;
  _Node<ElementType> _in_memory_node;
  size_t _size;
  size_t _T_parameter;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
BTreeList<ElementType>::BTreeList(size_t size, size_t limit, size_t T_parameter)
  : _size(size),
    _file_manager("./data"),
    _T_parameter(T_parameter) {
  _in_memory_node._info._elements_cnt = 0;
  _in_memory_node._parent_link = -1;
  _file_manager.SetNode(0, _in_memory_node);
}

template <typename ElementType>
void BTreeList<ElementType>::Insert(
    const unsigned int index,
    const ElementType& e
) {
  unsigned int curr_file_pos = _root_link;
  unsigned int indexes_on_the_left = 0;

  std::vector<unsigned int> path_indexes;
  path_indexes.push_back(_root_link);

  _Node<ElementType> curr_node;

  bool found = false;
  unsigned int in_node_index;

  do {
    curr_node = _file_manager.GetNode(curr_file_pos);
    in_node_index = 0;
    while (
      indexes_on_the_left + curr_node._children_cnts[in_node_index] + 1 < index
    ) {
      indexes_on_the_left += curr_node._children_cnts[in_node_index] + 1;
      ++in_node_index;
    }
    if (!curr_node.IsList()) {
      curr_file_pos = curr_node._children_cnts[in_node_index];
      path_indexes.push_back(curr_file_pos);
    } else {
      found = true;
    }
  } while (!found);

  bool finished = false;

  ElementType element_to_insert = e;
  unsigned int link_after_inserted = 0;
  unsigned int link_before_inserted = 0;


  while(!finished) {
    curr_node.Insert(in_node_index, element_to_insert);
    curr_node.SetLink(link_before_inserted, in_node_index); // Just change link
    curr_node.InsertLink(element_to_insert, in_node_index + 1);

    if (curr_node._elements.size() == _T_parameter * 2 - 1) {  // insertion into regular node
      if (!path_indexes.empty()) {
        element_to_insert = curr_node.GetMiddleElement();
        _Node<ElementType> new_node_to_add = curr_node.NodeFromSecondHalf();
        curr_node = curr_node.NodeFromFirstHalf();
        link_after_inserted = _file_manager.NewNode(new_node_to_add);
        link_before_inserted = *(path_indexes.rbegin());
        path_indexes.pop_back();
        curr_node = _file_manager.GetNode(*(path_indexes.rbegin()));
        // TODO: unroot or not to use root flags
      } else { // gone upper then root
        _Node<ElementType> new_root;
        new_root.Insert(element_to_insert, 0);
        new_root.InsertLink(link_before_inserted, 0);
        new_root.InsertLink(link_after_inserted, 1);
        new_root.SetIsRoot(true);
        _root_link = _file_manager.NewNode(new_root);
        finished = true;
      }
    } else {
      finished = true;
    }
  }
}

template<typename ElementType>
size_t BTreeList<ElementType>::Size() const {
  return _size;
}

#endif //B_TREE_LIST_LIBRARY_H