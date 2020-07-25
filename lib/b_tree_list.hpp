#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
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

  BTreeList(size_t size = 0, size_t limit = 256, size_t T_parameter = 200);

  void Insert(const unsigned int index, const ElementType& e);

  ElementType Extract(const unsigned int index);

  void Set(const unsigned int, const ElementType& e);

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

  //////////////////////////////////////////////////////////////////////////////
  // Private methods                                                          //
  //////////////////////////////////////////////////////////////////////////////

  void _FindPathToLeafByIndex(const unsigned int index,
                              std::vector<unsigned int> &file_pos_path,
                              std::vector<unsigned int> &in_node_indexes_path);

  _Node<ElementType> _FindElement(const unsigned int index,
                    unsigned int &file_pos,
                    unsigned int &index_to_operate);

  void _FindAppropriateInLeafElement(unsigned int &in_node_index,
                                     unsigned int &file_pos,
                                     unsigned int &in_parent_index,
                                     unsigned int &parent_file_pos);

  ElementType _ExtractFromLeaf(unsigned int in_node_index,
                               std::vector<unsigned int> &file_pos_path,
                               std::vector<unsigned int> &in_node_indexes_path);

};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
BTreeList<ElementType>::BTreeList(size_t size, size_t limit, size_t T_parameter)
  : _size(size),
    _file_manager(std::string{"./data"}, T_parameter),
    _T_parameter(T_parameter) {
  _in_memory_node._info._elements_cnt = 0;
  _in_memory_node._info._parent_link = -1;
  _file_manager.SetNode(0, _in_memory_node);
}

template <typename ElementType>
void BTreeList<ElementType>::Insert(
    const unsigned int index,
    const ElementType& e
) {
  std::vector<unsigned int> file_pos_path;
  std::vector<unsigned int> in_node_index_path;
  unsigned int in_node_index;
  _FindPathToLeafByIndex(index, file_pos_path, in_node_index_path);

  ElementType element_to_insert = e;
  unsigned int link_after_inserted = 0;
  unsigned int link_before_inserted = 0;

  do {
    unsigned int curr_pos = file_pos_path.back();
    unsigned int curr_in_node_index = in_node_index_path.back();
    file_pos_path.pop_back();
    in_node_index_path.pop_back();

    _Node<ElementType> curr_node = _file_manager.GetNode(curr_pos);

    curr_node.Insert(element_to_insert, curr_in_node_index);
    curr_node.SetLink(link_before_inserted, curr_in_node_index); // Just change link
    curr_node.InsertLink(element_to_insert, curr_in_node_index + 1);

    if (curr_node._elements.size() == _T_parameter * 2) {
      if (!file_pos_path.empty()) {
        element_to_insert = curr_node.GetMiddleElement();
        _Node<ElementType> new_node_to_add = curr_node.NodeFromSecondHalf();
        curr_node = curr_node.NodeFromFirstHalf();
        _file_manager.SetNode(curr_pos, curr_node);
        link_before_inserted = curr_pos;
        link_after_inserted = _file_manager.NewNode(new_node_to_add);
      } else { // gone upper then root
        _Node<ElementType> new_root;
        new_root.Insert(element_to_insert, 0);
        new_root.InsertLink(link_before_inserted, 0);
        new_root.InsertLink(link_after_inserted, 1);
        new_root.SetIsRoot(true);
        _root_link = _file_manager.NewNode(new_root);
      }
    } else {
      _file_manager.SetNode(curr_pos, curr_node);
      break;  // No need to go upper
    }
  } while (!file_pos_path.empty());
}

template <typename ElementType>
void BTreeList<ElementType>::Set(const unsigned int index, const ElementType& e) {
  unsigned int file_pos;
  unsigned int in_node_index;

  _Node<ElementType> node = _FindElement(index, file_pos, in_node_index);
  node.Set(e, in_node_index);
  _file_manager.SetNode(file_pos, node);
}

template <typename ElementType>
ElementType BTreeList<ElementType>::Get(const unsigned int index) {
  unsigned int file_pos;
  unsigned int in_node_index;

  _Node<ElementType> node = _FindElement(index, file_pos, in_node_index);
  return node._elements[in_node_index];
}

template<typename ElementType>
ElementType BTreeList<ElementType>::Extract(const unsigned int index) {
  unsigned int file_pos;
  unsigned int in_node_index;

  _Node<ElementType> node_to_extract =
      _FindElement(index, file_pos, in_node_index);




  if (node_to_extract.IsLeaf()) {
    // just extract this element from leaf
  } else {
    // extract element after this and set extracted equal to this
  }

}

template <typename ElementType>
size_t BTreeList<ElementType>::Size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////////////////
// Private methods                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
 * Function finds path to leaf to insert element into leaf (or extract it from
 * leaf).
 * Path are all indexes we chose to go down.
 * Last element of path is element, after which we insert element into leaf.
 *
 * In case of using for extracting, this func gives undefined behaviour if is
 * used for extracting element which is not inside any leaf. It will give
 * element index from leaf which is out of range.
 */

template <typename ElementType>
void BTreeList<ElementType>::_FindPathToLeafByIndex(
    const unsigned int index,
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path
) {
  unsigned int curr_file_pos = _root_link;
  file_pos_path.push_back(_root_link);
  bool found = false;
  unsigned int elements_to_skip = index;
  do {
    _Node<ElementType> curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned int in_node_index = 0;
    while (
        in_node_index < curr_node._children_cnts.size() &&
        elements_to_skip - curr_node._children_cnts[in_node_index] - 1 >= 0
    ) {
      elements_to_skip -= curr_node._children_cnts[in_node_index] + 1;
      ++in_node_index;
    }
    in_node_indexes_path.push_back(in_node_index);
    if (!curr_node.IsLeaf()) {
      curr_file_pos = curr_node._links[in_node_index];
      file_pos_path.push_back(curr_file_pos);
    } else {  // Gained leaf
      found = true;
    }
  } while (!found);
}

/*
 * Finds node and position in tree to set/get element.
 */

template <typename ElementType>
_Node<ElementType> BTreeList<ElementType>::_FindElement(
    const unsigned int index,
    unsigned int &file_pos,
    unsigned int &index_to_operate
) {
  unsigned int curr_file_pos = _root_link;
  bool found = false;
  unsigned int elements_to_skip = index;
  _Node<ElementType> curr_node;
  do {
    curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned int in_node_index = 0;
    while (
        in_node_index < curr_node._children_cnts.size() &&
        elements_to_skip - curr_node._children_cnts[in_node_index] - 1 >= 0
    ) {
      elements_to_skip -= curr_node._children_cnts[in_node_index] + 1;
      ++in_node_index;
    }
    if (elements_to_skip == curr_node._children_cnts[in_node_index]) {
      index_to_operate = in_node_index + 1;  // TODO: Check if it is true
      file_pos = curr_file_pos;
      found = true;
    } else {
      curr_file_pos = curr_node._links[in_node_index];  // TODO: Check if it is true
    }
  } while (!found);
  return curr_node;
}

/*
 * Changes parameteres to element to extract from leaf.
 */


template <typename ElementType>
void BTreeList<ElementType>::_FindAppropriateInLeafElement(
    unsigned int &in_node_index,
    unsigned int &file_pos,
    unsigned int &in_parent_index,
    unsigned int &parent_file_pos) {

  _Node<ElementType> curr_node = _file_manager.GetNode(file_pos);
  while (!curr_node.IsLeaf()) {
    parent_file_pos = file_pos;
    in_parent_index = in_node_index + 1;
    file_pos = curr_node._links[in_node_index] + 1;
    in_node_index = 0;
  }
}

template <typename ElementType>
ElementType BTreeList<ElementType>::_ExtractFromLeaf(
    unsigned int in_node_index,
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path
) {
  unsigned int leaf_file_pos = file_pos_path.back();
  file_pos_path.pop_back();
  _Node<ElementType> leaf_node = _file_manager.GetNode(leaf_file_pos);
  in_node_index = in_node_indexes_path.back();
  in_node_indexes_path.pop_back();

  ElementType element_to_return;

  if (leaf_node._elements.size() >= _T_parameter) {
    leaf_node.ExtractLink(in_node_index);
    element_to_return = leaf_node.ExtractElement(in_node_index);
  } else {  // leaf_node._elements.size() == _T_parameter - 1
    unsigned int parent_file_pos = file_pos_path.back();
    _Node<ElementType> parent_node = _file_manager.GetNode(parent_file_pos);
    unsigned int in_parent_index = in_node_indexes_path.back();
    in_node_indexes_path.pop_back();

    if (in_parent_index == 0) {  // With right neighbour
      unsigned int neighbour_pos = parent_node._links[1];
      _Node<ElementType> neighbour_node = _file_manager.GetNode(neighbour_pos);
      if (neighbour_node._elements.size() == _T_parameter - 1) { // connect
        leaf_node.ConnectWith(parent_node._elements[0], neighbour_node);
        _file_manager.SetNode(leaf_file_pos, leaf_node);
        parent_node.ExtractLink(0);
        parent_node.Extract(0);
        parent_node.SetLink(0, leaf_file_pos);
      } else { // move element
        ElementType element_to_move = parent_node.Extract(0);
        unsigned int link_to_move = parent_node.ExtractLink(0);
        unsigned int children_cnt_to_move = parent_node.ExtractChildrenCnt(0);

        leaf_node._elements.push_back(element_to_move);
        leaf_node._links.push_back(link_to_move);
        leaf_node._children_cnts.push_back(children_cnt_to_move);

        element_to_move = neighbour_node.Extract(0);
        link_to_move = neighbour_node.ExtractLink(0);
        children_cnt_to_move = neighbour_node.ExtractChildrenCnt(0);

        parent_node.Insert(element_to_move, 0);
        parent_node.InsertLink(link_to_move, 0);
        parent_node.InsertChildrenCnt(children_cnt_to_move, 0);

        _file_manager.DeleteNode(neighbour_pos);
        //set nodes
      }
    } else {  // With left neighbour
      _Node<ElementType> neighbour_node =
          _file_manager.GetNode(parent_node._links[in_parent_index - 1]);
      if (neighbour_node._elements.size() == _T_parameter - 1) { // connect
        neighbour_node.ConnectWith(parent_node._elements[in_parent_index - 1],
                                   leaf_node);
        _file_manager.SetNode(leaf_file_pos, leaf_node);
        parent_node.ExtractLink(in_parent_index - 1);
        parent_node.ExtractChildrenCnt(in_parent_index - 1);
        parent_node.Extract(in_parent_index - 1);
      } else { // move element
        ElementType element_to_move = parent_node.ExtractBack();
        unsigned int link_to_move = parent_node.ExtractBackLink();
        unsigned int children_cnt_to_move = parent_node.ExtractBackChildrenCnt();

        leaf_node._elements.push_back(element_to_move);
        leaf_node._links.push_back(link_to_move);
        leaf_node._children_cnts.push_back(children_cnt_to_move);

        element_to_move = neighbour_node.ExtractBack();
        link_to_move = neighbour_node.ExtractBackLink();
        children_cnt_to_move = neighbour_node.ExtractBackChildrenCnt();

        parent_node._elements.push_back(element_to_move);
        parent_node._links.push_back(link_to_move);
        parent_node._children_cnts.push_back(children_cnt_to_move);
      }
    }
  }

  return element_to_return;
}

#endif //B_TREE_LIST_LIBRARY_H