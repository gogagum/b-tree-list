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

//#define DEBUG

////////////////////////////////////////////////////////////////////////////////
// Subsidiary classes                                                         //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
class BTreeList{
 public:

  BTreeList(size_t size = 0, size_t limit = 256, size_t T_parameter = 200);

  void Insert(const unsigned int index, const ElementType& e);

  ElementType Extract(const unsigned int index);

  void Set(const unsigned int index, const ElementType& e);

  ElementType Get(const unsigned int index);

  size_t Size() const;

  #ifdef DEBUG
  void print_all_nodes();
  #endif
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

  _Node<ElementType> _FindElement(const unsigned int index,
                                  unsigned int &file_pos,
                                  unsigned int &index_to_operate);

  void _FindPathToLeafByIndex(const unsigned int index,
                              std::vector<unsigned int> &file_pos_path,
                              std::vector<unsigned int> &in_node_indexes_path);

  _Node<ElementType> _FindPathByIndex(const unsigned int index,
                        std::vector<unsigned int> &file_pos_path,
                        std::vector<unsigned int> &in_node_indexes_path);

  void _FindAppropriateInLeafElement(
      std::vector<unsigned int> &file_pos_path,
      std::vector<unsigned int> &in_node_indexes_path
  );

  ElementType _ExtractFromLeaf(std::vector<unsigned int> &file_pos_path,
                               std::vector<unsigned int> &in_node_indexes_path);

  bool _CorrectNodeOnExtract(unsigned int file_pos,
                             unsigned int parent_file_pos,
                             unsigned int in_parent_pos);

  void _CorrectChildrenCnts(std::vector<unsigned int> &file_pos_path,
                            std::vector<unsigned int> &in_node_indexes_path,
                            int to_change);

  void _Rebuild();
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
BTreeList<ElementType>::BTreeList(size_t size, size_t limit, size_t T_parameter)
  : _size(size),
    _file_manager(std::string{"./data"}, T_parameter),
    _T_parameter(T_parameter),
    _root_link(0),
    _in_memory_node() {
  _in_memory_node.SetIsRoot(true);
  _in_memory_node.SetIsLeaf(true);
  _in_memory_node.SetIsRightestLeaf(true);
  _file_manager.SetNode(0, _in_memory_node);
}

template <typename ElementType>
void BTreeList<ElementType>::Insert(const unsigned int index,
                                    const ElementType &e) {
  ++_size;
  std::vector<unsigned int> file_pos_path;
  std::vector<unsigned int> in_node_index_path;
  _FindPathToLeafByIndex(index, file_pos_path, in_node_index_path);

  ElementType element_to_insert = e;
  unsigned int link_after_inserted = 0;
  unsigned int link_before_inserted = 0;
  unsigned int children_cnt_after_inserted = 0;
  unsigned int children_cnt_before_inserted = 0;

  do {
    unsigned int curr_file_pos = file_pos_path.back();
    unsigned int curr_innode_index = in_node_index_path.back();
    file_pos_path.pop_back();
    in_node_index_path.pop_back();

    _Node<ElementType> curr_node = _file_manager.GetNode(curr_file_pos);

    curr_node.Insert(curr_innode_index, element_to_insert);
    curr_node.LinkBefore(curr_innode_index) = link_before_inserted; // Just change link
    curr_node.InsertLink(curr_innode_index + 1, link_after_inserted);
    curr_node.ChildrenCntBefore(curr_innode_index) =
        children_cnt_before_inserted;
    curr_node.InsertChildrenCnt(
        curr_innode_index + 1, children_cnt_after_inserted);

    if (curr_node.Size() == _T_parameter * 2 - 1) {
      element_to_insert = curr_node.GetMiddleElement();
      // Prepare halves
      _Node<ElementType> first_half_node = curr_node.NodeFromFirstHalf();
      _Node<ElementType> second_half_node = curr_node.NodeFromSecondHalf();
      // Prepare links
      link_before_inserted = curr_file_pos;
      link_after_inserted = _file_manager.NewNode();
      // Prepare children cnts
      children_cnt_before_inserted = first_half_node.GetAllChildrenCnt();
      children_cnt_after_inserted = second_half_node.GetAllChildrenCnt();
      // Now they cannot be leaves
      first_half_node.SetIsRoot(false);
      second_half_node.SetIsRoot(false);
      // Set halves
      _file_manager.SetNode(link_before_inserted, first_half_node);
      _file_manager.SetNode(link_after_inserted, second_half_node);
      if (file_pos_path.empty()) { // gone upper then root
        _Node<ElementType> new_root(
            std::vector<ElementType>{element_to_insert},
            std::vector<unsigned int>{link_before_inserted,
                                      link_after_inserted},
            std::vector<unsigned int>{children_cnt_before_inserted,
                                      children_cnt_after_inserted},
            _Node<ElementType>::_Flags::ROOT
        );
        _root_link = _file_manager.NewNode();
        _file_manager.SetNode(_root_link, new_root);
      }
    } else {  // Just inserted
      _file_manager.SetNode(curr_file_pos, curr_node);
      _CorrectChildrenCnts(file_pos_path, in_node_index_path, 1);
      break;  // No need to go upper
    }
  } while (!file_pos_path.empty());

}

template <typename ElementType>
void BTreeList<ElementType>::Set(const unsigned int index, const ElementType& e) {
  unsigned int file_pos = _root_link;
  unsigned int in_node_index;

  _Node<ElementType> node = _FindElement(index, file_pos, in_node_index);
  node.IthElement(in_node_index) = e;
  _file_manager.SetNode(file_pos, node);
}

template <typename ElementType>
ElementType BTreeList<ElementType>::Get(const unsigned int index) {
  unsigned int file_pos = _root_link;
  unsigned int in_node_index;

  _Node<ElementType> node = _FindElement(index, file_pos, in_node_index);
  return node._elements[in_node_index];
}

template<typename ElementType>
ElementType BTreeList<ElementType>::Extract(const unsigned int index) {
  --_size;
  std::vector<unsigned int> file_pos_path;
  std::vector<unsigned int> in_node_indexes_path;

  ElementType element_to_extract;

  _FindPathByIndex(index, file_pos_path, in_node_indexes_path);
  _Node<ElementType> node_to_extract =
      _file_manager.GetNode(file_pos_path.back());

  if (node_to_extract.GetIsLeaf()) {
    element_to_extract = _ExtractFromLeaf(file_pos_path, in_node_indexes_path);
  } else {
    _Node<ElementType> node_to_change =
        _file_manager.GetNode(file_pos_path.back());
    element_to_extract = node_to_change.IthElement(in_node_indexes_path.back());
    unsigned int pos_to_save = file_pos_path.back();
    _FindAppropriateInLeafElement(file_pos_path, in_node_indexes_path);
  }
  return element_to_extract;
}

template <typename ElementType>
size_t BTreeList<ElementType>::Size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////////////////
// Private methods                                                            //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
_Node<ElementType> BTreeList<ElementType>::_FindElement(
    const unsigned int index,
    unsigned int &file_pos,
    unsigned int &index_to_operate
) {
  bool found = false;
  int elements_to_skip = index;
  _Node<ElementType> curr_node;
  do {
    curr_node = _file_manager.GetNode(file_pos);
    unsigned int in_node_index = 0;
    while (
        in_node_index < curr_node._elements.size() &&
            (elements_to_skip -
                static_cast<int>(curr_node._children_cnts[in_node_index]) - 1) >= 0
        ) {
      elements_to_skip -= (curr_node._children_cnts[in_node_index] + 1);
      ++in_node_index;
    }
    if (in_node_index < curr_node._elements.size() &&
        elements_to_skip == curr_node._children_cnts[in_node_index]) {
      index_to_operate = in_node_index;  // TODO: Check if it is true
      found = true;
    } else {
      file_pos = curr_node._links[in_node_index];  // TODO: Check if it is true
    }
  } while (!found);
  return curr_node;
}

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
  int elements_to_skip = static_cast<int>(index);
  do {
    _Node<ElementType> curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned int in_node_index = 0;
    while (
        in_node_index < curr_node._elements.size() &&
        elements_to_skip -
        static_cast<int>(curr_node.ChildrenCntBefore(in_node_index)) - 1 >= 0
    ) {
      elements_to_skip -= curr_node.ChildrenCntBefore(in_node_index) + 1;
      ++in_node_index;
    }
    in_node_indexes_path.push_back(in_node_index);
    if (!curr_node.GetIsLeaf()) {
      curr_file_pos = curr_node._links[in_node_index];
      file_pos_path.push_back(curr_file_pos);
    } else {  // Gained leaf
      found = true;
    }
  } while (!found);
}

/*
 * Finds node and position in tree to set/get element.
 * Gets file pos as hint to start searching from it.
 */

template <typename ElementType>
_Node<ElementType> BTreeList<ElementType>::_FindPathByIndex(
    const unsigned int index,
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path
) {
  unsigned int curr_file_pos = _root_link;
  file_pos_path.push_back(_root_link);
  bool found = false;
  int elements_to_skip = static_cast<int>(index);
  _Node<ElementType> curr_node;
  do {
    curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned int in_node_index = 0;
    while (
        in_node_index < curr_node.Size() &&
        elements_to_skip -
        static_cast<int>(curr_node.ChildrenCntBefore(in_node_index)) - 1 >= 0
    ) {
      elements_to_skip -= (curr_node.ChildrenCntBefore(in_node_index) + 1);
      ++in_node_index;
    }
    in_node_indexes_path.push_back(in_node_index);
    if (
        in_node_index < curr_node.Size() &&
        elements_to_skip == curr_node.ChildrenCntBefore(in_node_index)
    ) {
      found = true;
    } else {
      file_pos_path.push_back(curr_node.LinkBefore(in_node_index));
      curr_node = _file_manager.GetNode(file_pos_path.back());
    }
  } while (!found);
  return curr_node;
}

/*
 * Changes parameters to element to extract from leaf.
 */

template <typename ElementType>
void BTreeList<ElementType>::_FindAppropriateInLeafElement(
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path
) {
  _Node<ElementType> curr_node = _file_manager.GetNode(file_pos_path.back());
  file_pos_path.push_back(curr_node.LinkAfter(in_node_indexes_path.back()));
  curr_node = _file_manager.GetNode(file_pos_path.back());
  in_node_indexes_path.push_back(0);
  while (!curr_node.GetIsLeaf()) {
    file_pos_path.push_back(curr_node.LinkBefore(0));
    curr_node = _file_manager.GetNode(file_pos_path.back());
    in_node_indexes_path.push_back(0);
  }
}

template <typename ElementType>
ElementType BTreeList<ElementType>::_ExtractFromLeaf(
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path
) {
  unsigned int leaf_file_pos = file_pos_path.back();
  file_pos_path.pop_back();
  _Node<ElementType> leaf_node = _file_manager.GetNode(leaf_file_pos);
  unsigned int in_node_index = in_node_indexes_path.back();
  in_node_indexes_path.pop_back();

  ElementType element_to_return = leaf_node.Extract(in_node_index);
  leaf_node.ExtractLinkBefore(in_node_index);
  leaf_node.ExtractChildrenCntBefore(in_node_index);

  if (leaf_node.Size() < _T_parameter) {
    unsigned int curr_file_pos = leaf_file_pos;
    _Node<ElementType> curr_node = leaf_node;
    if (!file_pos_path.empty()) {
      unsigned int parent_file_pos = file_pos_path.back();
      unsigned int in_parent_index = in_node_indexes_path.back();
      while (_CorrectNodeOnExtract(curr_file_pos, parent_file_pos, in_parent_index)) {
        curr_file_pos = parent_file_pos;
        parent_file_pos = file_pos_path.back();  // TODO: accuracy
        in_parent_index = in_node_indexes_path.back();
        file_pos_path.pop_back();
        in_node_indexes_path.pop_back();
      }
    }
  }
  _CorrectChildrenCnts(file_pos_path, in_node_indexes_path, -1);
  return element_to_return;
}

template <typename ElementType>
bool BTreeList<ElementType>::_CorrectNodeOnExtract(
    unsigned int file_pos,
    unsigned int parent_file_pos,
    unsigned int in_parent_index
) {
  bool finished = false;

  _Node<ElementType> node = _file_manager.GetNode(file_pos);
  _Node<ElementType> parent_node = _file_manager.GetNode(parent_file_pos);

  unsigned int neighbour_node_file_pos;
  _Node<ElementType> neighbour_node;
  bool with_left = true;

  if (in_parent_index == 0) {
    neighbour_node_file_pos = parent_node.LinkAfter(in_parent_index);
    neighbour_node = _file_manager.GetNode(neighbour_node_file_pos);
    with_left = false;
  } else {  // in_parent_index > 0
    neighbour_node_file_pos = parent_node.LinkBefore(in_parent_index);
    neighbour_node = _file_manager.GetNode(neighbour_node_file_pos);
  }

  if (neighbour_node.Size() == _T_parameter - 1) {  // connect
    _Node<ElementType> connected_node;
    if (with_left) {
      connected_node =
          Connect(neighbour_node, node,
                  parent_node.Extract(in_parent_index - 1));
      parent_node.ExtractLinkBefore(in_parent_index - 1);
      parent_node.ExtractChildrenCntBefore(in_parent_index - 1);
    } else {  // with right
      connected_node =
          Connect(node, neighbour_node, parent_node.Extract(in_parent_index));
      parent_node.ExtractLinkAfter(in_parent_index);
      parent_node.ExtractChildrenCntAfter(in_parent_index);
    }
    parent_node.ChildrenCntBefore(in_parent_index) =
        connected_node.GetAllChildrenCnt();
    _file_manager.DeleteNode(neighbour_node_file_pos);
    _file_manager.SetNode(file_pos, connected_node);
    _file_manager.SetNode(parent_file_pos, parent_node);
    if (parent_node.Size() >= _T_parameter - 1 ||
        parent_node.GetIsRoot()) {
      finished = true;
    }
  } else {  // move element
    if (with_left) {
      ElementType element_from_neighbour = neighbour_node.ExtractBack();
      unsigned int link_from_neighbour = neighbour_node.ExtractBackLink();
      unsigned int cc_from_neighbour = neighbour_node.ExtractBackChildrenCnt();
      ElementType element_from_parent =
          parent_node.IthElement(in_parent_index - 1);
      node.InsertTrio(0, element_from_parent,
                      link_from_neighbour, cc_from_neighbour);
      parent_node.IthElement(in_parent_index - 1) = element_from_neighbour;
      parent_node.ChildrenCntBefore(in_parent_index - 1) =
          neighbour_node.GetAllChildrenCnt();
      parent_node.ChildrenCntAfter(in_parent_index - 1) =
          node.GetAllChildrenCnt();
    } else {  // with right
      ElementType element_from_neighbour = neighbour_node.Extract(0);
      unsigned int link_from_neighbour = neighbour_node.ExtractLinkBefore(0);
      unsigned int cc_from_neighbour = neighbour_node.ExtractChildrenCntBefore(0);
      ElementType element_from_parent = parent_node.IthElement(in_parent_index);
      node.PushBackTrio(element_from_parent,
                        link_from_neighbour, cc_from_neighbour);
      parent_node.IthElement(in_parent_index) = element_from_neighbour;
      parent_node.ChildrenCntBefore(in_parent_index) =
          node.GetAllChildrenCnt();
      parent_node.ChildrenCntAfter(in_parent_index) =
          neighbour_node.GetAllChildrenCnt();
    }
    _file_manager.SetNode(parent_file_pos, parent_node);
    _file_manager.SetNode(file_pos, node);
    _file_manager.SetNode(neighbour_node_file_pos, neighbour_node);
    finished = true;  // After moving everything is always OK
  }
  return finished;
}

template <typename ElementType>
void BTreeList<ElementType>::_CorrectChildrenCnts(
    std::vector<unsigned int> &file_pos_path,
    std::vector<unsigned int> &in_node_indexes_path,
    int to_change
) {
  while (!file_pos_path.empty()) {
    _Node<ElementType> node_to_correct =
        _file_manager.GetNode(file_pos_path.back());
    node_to_correct.SetChildrenCntBefore(in_node_indexes_path.back()) +=
        to_change;
    _file_manager.SetNode(file_pos_path.back(), node_to_correct);
    file_pos_path.pop_back();
    in_node_indexes_path.pop_back();
  }
}

template <typename ElementType>
void BTreeList<ElementType>::_Rebuild() {
  _FileSavingManager<ElementType> new_file_manager("data_tmp", _T_parameter);
  unsigned int last_added_node_pos = 1;
  unsigned int curr_pos = 0;
  new_file_manager.NewNode();
  new_file_manager.SetNode(curr_pos, _file_manager.GetNode(_root_link));
  do {
    _Node<ElementType> curr_node = new_file_manager.GetNode(curr_pos);
    if (!curr_node.GetIsLeaf()) {
      for (unsigned int i = 0; i < curr_node.Size() + 1; ++i) {
        _Node<ElementType> node_to_copy =
            _file_manager.GetNode(curr_node.LinkBefore(i));
        unsigned int pos_to_set_node = new_file_manager.NewNode();
        new_file_manager.SetNode(pos_to_set_node, node_to_copy);
        ++last_added_node_pos;
        curr_node.LinkBefore(i) = pos_to_set_node;
      }
      new_file_manager.SetNode(curr_pos, curr_node);
    }
    ++curr_pos;
  } while (curr_pos < last_added_node_pos);
  // delete old file manager
  // rename file and change _file_manager
}

#ifdef DEBUG
template <typename ElementType>
void BTreeList<ElementType>::print_all_nodes() {
  std::cout << "Nodes:\n";
  for (int i = 0; i < _file_manager._allocator._data_info._max_blocks_cnt; ++i) {
    _Node<ElementType> node = _file_manager.GetNode(i);
    for (auto j: node._elements) {
      std::cout << j << ' ';
    }
    std::cout << '\n';
    for (auto j: node._links) {
      std::cout << j << ' ';
    }
    std::cout << '\n';
    for (auto j: node._children_cnts) {
      std::cout << j << ' ';
    }
    std::cout << "\n";
    if (node.GetIsLeaf()) {
      std::cout << "Leaf. \n";
    }
    if (node.GetIsRoot()) {
      std::cout << "Root. \n";
    }
    std::cout << '\n';
  }
}
#endif

#endif //B_TREE_LIST_LIBRARY_H