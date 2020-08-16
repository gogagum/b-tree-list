#include <fcntl.h>
#include <string>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <sys/mman.h>
#include "node.hpp"
#include "file_saving_manager.hpp"
#include "data_info.hpp"
#include "block_rw.hpp"

#ifndef B_TREE_LIST_LIBRARY_H
#define B_TREE_LIST_LIBRARY_H

template <typename ElementType, size_t T = 200>
class BTreeList{
 public:

  // Simple constructor.
  // If file with filename name exists, tries open it as data file.
  // If not exists creates new empty file.
  explicit BTreeList(const std::string &filename, bool rebuild_flag = true);

  // Creates file for tree of size size
  // If file with such name exists truncates it
  template <typename SizeType>
  BTreeList(const std::string &filename,
            SizeType size,
            bool rebuild_flag = true);

  // Creates file for tree of size size and fills it with element_to_fill.
  // If file with such name exists truncates it.
  BTreeList(const std::string &filename,
            size_t size,
            const ElementType &element_to_fill,
            bool rebuild_flag = true);

  // Creates file of size suitable to iterators given
  // If file with such name exists truncates it.
  template <typename IteratorType>
  BTreeList(const std::string &filename,
            IteratorType begin, IteratorType end,
            bool rebuild_flag = true);

  // Insert element to index position.
  void Insert(unsigned index, const ElementType& e);

  // Insert elements from iterators range starting with index position.
  template <typename IteratorType>
  void Insert(unsigned index, IteratorType begin, IteratorType end);

  // Extract element from index position
  ElementType Extract(unsigned index);

  // Access to element by index
  ElementType& operator[](unsigned index);

  // Access to element by index
  ElementType operator[](unsigned index) const;

  // Get size of structure
  [[nodiscard]] size_t Size() const;

  ~BTreeList();

 private:
  //////////////////////////////////////////////////////////////////////////////
  // Private fields                                                           //
  //////////////////////////////////////////////////////////////////////////////

  std::shared_ptr<DataInfo> _data_info_ptr;

  FileSavingManager<ElementType, T> _file_manager;

  bool _rebuild_flag;

  //////////////////////////////////////////////////////////////////////////////
  // Private methods                                                          //
  //////////////////////////////////////////////////////////////////////////////

  void _ResizeFromEmpty(size_t size);

  void _ResizeFromEmpty(size_t size, const ElementType &element_to_fill);

  template <typename IteratorType>
  void _Insert(unsigned &index, IteratorType &begin, IteratorType &end);

  void _AllocateBackElements(size_t &cnt,
                             const ElementType& element_to_fill,
                             bool need_to_set_flag);

  unsigned _FindInNodeIndex(const Node<ElementType, T> &node,
                            int64_t &elements_to_skip);

  Node<ElementType, T> _FindElement(unsigned index,
                                    file_pos_t &file_pos,
                                    unsigned &index_to_operate);

  void _FindPathToLeafByIndex(unsigned index,
                              std::vector<file_pos_t> &file_pos_path,
                              std::vector<unsigned> &indexes_path);

  Node<ElementType, T> _FindPathByIndex(
      unsigned index,
      std::vector<file_pos_t> &file_pos_path,
      std::vector<unsigned> &indexes_path
  );

  void _FindAppropriateInLeafElement(
      std::vector<file_pos_t > &file_pos_path,
      std::vector<unsigned> &indexes_path
  );

  ElementType _ExtractFromLeaf(std::vector<file_pos_t> &file_pos_path,
                               std::vector<unsigned> &indexes_path);

  void _MoveElementFromLeftNeighbour(
      Node<ElementType, T> &node,
      Node<ElementType, T> &neighbour_node,
      Node<ElementType, T> &parent_node,
      unsigned &in_parent_index
  );

  void _MoveElementFromRightNeighbour(
      Node<ElementType, T> &node,
      Node<ElementType, T> &neighbour_node,
      Node<ElementType, T> &parent_node,
      unsigned &in_parent_index
  );

  bool _CorrectNodeOnExtract(Node<ElementType, T> &node,
                             Node<ElementType, T> &parent_node,
                             file_pos_t file_pos,
                             file_pos_t parent_file_pos,
                             unsigned in_parent_index);

  void _CorrectChildrenCnts(std::vector<file_pos_t> &file_pos_path,
                            std::vector<unsigned> &in_node_indexes_path,
                            int to_change);

  void _Rebuild();
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
BTreeList<ElementType, T>::BTreeList(const std::string &filename,
                                     bool rebuild_flag)
    : _data_info_ptr(std::make_shared<DataInfo>()),
      _file_manager(filename, _data_info_ptr, false),
      _rebuild_flag(rebuild_flag) {}

template <typename ElementType, size_t T>
template <typename SizeType>
BTreeList<ElementType, T>::BTreeList(const std::string &filename,
                                     SizeType size,
                                     bool rebuild_flag)
  : _data_info_ptr(std::make_shared<DataInfo>()),
    _file_manager(filename, _data_info_ptr, true),
    _rebuild_flag(rebuild_flag) {
  _ResizeFromEmpty(size);
}

template <typename ElementType, size_t T>
BTreeList<ElementType, T>::BTreeList(const std::string &filename,
                                     size_t size,
                                     const ElementType& element,
                                     bool rebuild_flag)
    : _data_info_ptr(std::make_shared<DataInfo>()),
      _file_manager(filename, _data_info_ptr, true),
      _rebuild_flag(rebuild_flag) {
  _ResizeFromEmpty(size, element);
}

template <typename ElementType, size_t T>
template <typename IteratorType>
BTreeList<ElementType, T>::BTreeList(const std::string &filename,
                                     IteratorType begin,
                                     IteratorType end,
                                     bool rebuild_flag)
  : _data_info_ptr(std::make_shared<DataInfo>()),
    _file_manager(filename, _data_info_ptr, true),
    _rebuild_flag(rebuild_flag) {
  Insert(0, begin, end);
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::Insert(unsigned index, const ElementType &e) {
  ++_data_info_ptr->_size;
  std::vector<file_pos_t> file_pos_path;
  std::vector<unsigned> indexes_path;
  _FindPathToLeafByIndex(index, file_pos_path, indexes_path);

  ElementType element_to_insert = e;
  file_pos_t link_after_inserted = 0;
  file_pos_t link_before_inserted = 0;
  size_t cc_after_inserted = 0;
  size_t cc_before_inserted = 0;

  do {
    file_pos_t curr_file_pos = file_pos_path.back();
    unsigned curr_innode_index = indexes_path.back();
    file_pos_path.pop_back();
    indexes_path.pop_back();

    Node<ElementType, T> curr_node = _file_manager.GetNode(curr_file_pos);

    curr_node.Insert(curr_innode_index, element_to_insert);
    curr_node.SetLinks(curr_innode_index,
                       link_before_inserted, link_after_inserted);
    curr_node.SetChildrenCnts(curr_innode_index,
                              cc_before_inserted, cc_after_inserted);

    if (curr_node.Size() >= T * 2 - 1) {
      element_to_insert = curr_node.GetMiddleElement();
      // Prepare halves
      Node<ElementType, T> first_half_node = curr_node.NodeFromFirstHalf();
      Node<ElementType, T> second_half_node = curr_node.NodeFromSecondHalf();
      // Prepare links
      link_before_inserted = curr_file_pos;
      link_after_inserted = _file_manager.NewNode();
      // Prepare children cnts
      cc_before_inserted = first_half_node.GetAllChildrenCnt();
      cc_after_inserted = second_half_node.GetAllChildrenCnt();
      // Set halves
      _file_manager.SetNode(link_before_inserted, first_half_node);
      _file_manager.SetNode(link_after_inserted, second_half_node);
      if (file_pos_path.empty()) { // Separated root
        Node<ElementType, T> new_root(
            std::vector<ElementType>{element_to_insert},
            std::vector<file_pos_t>{link_before_inserted, link_after_inserted},
            std::vector<size_t>{cc_before_inserted, cc_after_inserted},
            Node<ElementType, T>::_Flags::ROOT
        );
        _data_info_ptr->_root_pos = _file_manager.NewNode(new_root);
      }
    } else {  // Just inserted
      _file_manager.SetNode(curr_file_pos, curr_node);
      _CorrectChildrenCnts(file_pos_path, indexes_path, 1);
    }
  } while (!file_pos_path.empty());
}

template<typename ElementType, size_t T>
template<typename IteratorType>
void BTreeList<ElementType, T>::Insert(unsigned index,
                                       IteratorType begin,
                                       IteratorType end) {
  while (begin != end) {
    _Insert(index, begin, end);
    if (begin != end) {
      Insert(index, *begin);
      ++begin;
      ++index;
    }
  }
}

template <typename ElementType, size_t T>
ElementType& BTreeList<ElementType, T>::operator[](unsigned index) {
  file_pos_t file_pos = _data_info_ptr->_root_pos;
  unsigned in_node_index;

  _FindElement(index, file_pos, in_node_index);
  return *_file_manager._block_rw.template GetNodeElementPtr<ElementType, T>(
      file_pos,
      in_node_index
  );
};

template <typename ElementType, size_t T>
ElementType BTreeList<ElementType, T>::operator[](unsigned index) const {
  file_pos_t file_pos = _data_info_ptr->_root_pos;
  unsigned in_node_index;

  _FindElement(index, file_pos, in_node_index);
  return *_file_manager._block_rw.template GetNodeElementPtr<ElementType,T>(
      file_pos,
      in_node_index
  );
};

//template <typename ElementType, size_t T>
//ElementType BTreeList<ElementType, T>::Get(unsigned index) {
//  file_pos_t file_pos = _data_info_ptr->_root_pos;
//  unsigned in_node_index;
//
//  Node<ElementType, T> node = _FindElement(index, file_pos, in_node_index);
//  return node._elements[in_node_index];
//}

template<typename ElementType, size_t T>
ElementType BTreeList<ElementType, T>::Extract(unsigned index) {
  --_data_info_ptr->_size;
  std::vector<file_pos_t> file_pos_path;
  std::vector<unsigned> indexes_path;

  _FindPathByIndex(index, file_pos_path, indexes_path);
  Node<ElementType, T> node_with_element =
      _file_manager.GetNode(file_pos_path.back());

  ElementType element_to_extract;
  if (node_with_element.GetIsLeaf()) {
    element_to_extract = _ExtractFromLeaf(file_pos_path, indexes_path);
  } else {
    element_to_extract = node_with_element.Element(indexes_path.back());
    _FindAppropriateInLeafElement(file_pos_path, indexes_path);
    ElementType element_from_leaf = _ExtractFromLeaf(file_pos_path,
                                                     indexes_path);
    (*this)[index] = element_from_leaf;
  }
  return element_to_extract;
}

template <typename ElementType, size_t T>
size_t BTreeList<ElementType, T>::Size() const {
  return _data_info_ptr->_size;
}

////////////////////////////////////////////////////////////////////////////////
// Private methods                                                            //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_ResizeFromEmpty(size_t size) {
  while (size != 0) {
    _AllocateBackElements(size, 0, false);
    if (size != 0) {
      Insert(Size() - 1, 0);
      --size;
    }
  }
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_ResizeFromEmpty(
    size_t size,
    const ElementType &element_to_fill
) {
  while (size != 0) {
    _AllocateBackElements(size, element_to_fill, true);
    if (size != 0) {
      Insert(Size() - 1, element_to_fill);
      --size;
    }
  }
}

template <typename ElementType, size_t T>
template <typename IteratorType>
void BTreeList<ElementType, T>::_Insert(
    unsigned &index,
    IteratorType &begin,
    IteratorType &end
) {
  std::vector<file_pos_t> file_pos_path;
  std::vector<unsigned> indexes_path;
  _FindPathToLeafByIndex(index, file_pos_path, indexes_path);

  file_pos_t leaf_file_pos = file_pos_path.back();
  file_pos_path.pop_back();

  auto leaf_node = _file_manager.GetNode(leaf_file_pos);
  unsigned elements_possible_to_insert = 2 * T - 2 - leaf_node.Size();
  unsigned elements_to_insert = 0;
  auto new_begin = begin;
  while (elements_to_insert < elements_possible_to_insert && new_begin != end) {
    ++new_begin;
    ++elements_to_insert;
  }
  leaf_node.Insert(indexes_path.back(), begin, new_begin);
  _file_manager.SetNode(leaf_file_pos, leaf_node);
  indexes_path.pop_back();
  begin = new_begin;
  index += elements_to_insert;
  _data_info_ptr->_size += elements_to_insert;
  _CorrectChildrenCnts(file_pos_path, indexes_path, elements_to_insert);
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_AllocateBackElements(
    size_t &cnt,
    const ElementType &element_to_fill,
    bool need_to_set_flag
) {
  std::vector<file_pos_t> file_pos_path;
  std::vector<unsigned> indexes_path;
  _FindPathToLeafByIndex(Size(), file_pos_path, indexes_path);

  file_pos_t leaf_file_pos = file_pos_path.back();
  file_pos_path.pop_back();
  indexes_path.pop_back();

  auto leaf_node = _file_manager.GetNode(leaf_file_pos);
  unsigned elements_to_allocate = std::min(2 * T - 2 - leaf_node.Size(), cnt);
  if (need_to_set_flag) {
    leaf_node.Resize(leaf_node.Size() + elements_to_allocate, element_to_fill);
  } else {
    leaf_node.Resize(leaf_node.Size() + elements_to_allocate);
  }
  _file_manager.SetNode(leaf_file_pos, leaf_node);
  cnt -= elements_to_allocate;
  _data_info_ptr->_size += elements_to_allocate;
  _CorrectChildrenCnts(file_pos_path, indexes_path, elements_to_allocate);
}

template <typename ElementType, size_t T>
unsigned  BTreeList<ElementType, T>::_FindInNodeIndex(
    const Node<ElementType, T> &node,
    int64_t &elements_to_skip
) {
  unsigned in_node_index = 0;
  while (
      in_node_index < node.Size() &&
      elements_to_skip -
      static_cast<int>(node.ChildrenCntBefore(in_node_index)) - 1 >= 0
  ) {
    elements_to_skip -= node.ChildrenCntBefore(in_node_index) + 1;
    ++in_node_index;
  }
  return in_node_index;
}

template <typename ElementType, size_t T>
Node<ElementType, T> BTreeList<ElementType, T>::_FindElement(
    unsigned index,
    file_pos_t &file_pos,
    unsigned &index_to_operate
) {
  bool found = false;
  auto elements_to_skip = static_cast<int64_t>(index);
  Node<ElementType, T> curr_node;
  do {
    curr_node = _file_manager.GetNode(file_pos);
    unsigned in_node_index = _FindInNodeIndex(curr_node, elements_to_skip);
    if (in_node_index < curr_node.Size() &&
        elements_to_skip == curr_node.ChildrenCntBefore(in_node_index)) {
      index_to_operate = in_node_index;
      found = true;
    } else {
      file_pos = curr_node.LinkBefore(in_node_index);
    }
  } while (!found);
  return curr_node;
}

/*
 * Function finds path to leaf to insert element into leaf (or extract it from
 * leaf).
 *
 * file_pos_path are all file positions of nodes which are visited including
 * leaf node and root node.
 *
 * indexes_path are all indexes we chose to go down.
 * Last element of path is element, after which we insert element into leaf.
 *
 * In case of using for extracting, this func gives undefined behaviour if is
 * used for extracting element which is not inside any leaf. It will give
 * element index from leaf which is out of range.
 */

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_FindPathToLeafByIndex(
    unsigned index,
    std::vector<file_pos_t> &file_pos_path,
    std::vector<unsigned> &indexes_path
) {
  file_pos_t curr_file_pos = _data_info_ptr->_root_pos;
  file_pos_path.push_back(curr_file_pos);
  bool found = false;
  auto elements_to_skip = static_cast<int64_t>(index);
  do {
    Node<ElementType, T> curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned in_node_index = _FindInNodeIndex(curr_node, elements_to_skip);
    indexes_path.push_back(in_node_index);
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

template <typename ElementType, size_t T>
Node<ElementType, T> BTreeList<ElementType, T>::_FindPathByIndex(
    unsigned index,
    std::vector<file_pos_t> &file_pos_path,
    std::vector<unsigned> &indexes_path
) {
  file_pos_t curr_file_pos = _data_info_ptr->_root_pos;
  file_pos_path.push_back(curr_file_pos);
  bool found = false;
  auto elements_to_skip = static_cast<int64_t>(index);
  Node<ElementType, T> curr_node;
  do {
    curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned in_node_index = _FindInNodeIndex(curr_node, elements_to_skip);
    indexes_path.push_back(in_node_index);
    if (
        in_node_index < curr_node.Size() &&
        elements_to_skip == curr_node.ChildrenCntBefore(in_node_index)
    ) {
      found = true;
    } else {
      curr_file_pos = curr_node.LinkBefore(in_node_index);
      file_pos_path.push_back(curr_file_pos);
    }
  } while (!found);
  return curr_node;
}

/*
 * Changes parameters to element to extract from leaf.
 */

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_FindAppropriateInLeafElement(
    std::vector<file_pos_t> &file_pos_path,
    std::vector<unsigned> &indexes_path
) {
  Node<ElementType, T> curr_node = _file_manager.GetNode(file_pos_path.back());
  ++indexes_path.back();
  file_pos_path.push_back(curr_node.LinkBefore(indexes_path.back()));
  indexes_path.push_back(0);
  curr_node = _file_manager.GetNode(file_pos_path.back());
  while (!curr_node.GetIsLeaf()) {
    file_pos_path.push_back(curr_node.LinkBefore(0));
    indexes_path.push_back(0);
    curr_node = _file_manager.GetNode(file_pos_path.back());
  }
}

template <typename ElementType, size_t T>
ElementType BTreeList<ElementType, T>::_ExtractFromLeaf(
    std::vector<file_pos_t> &file_pos_path,
    std::vector<unsigned> &indexes_path
) {
  file_pos_t curr_file_pos = file_pos_path.back();
  unsigned in_node_index = indexes_path.back();
  file_pos_path.pop_back();
  indexes_path.pop_back();

  Node<ElementType, T> curr_node = _file_manager.GetNode(curr_file_pos);

  ElementType element_to_return = curr_node.Extract(in_node_index);
  curr_node.ExtractLinkBefore(in_node_index);
  curr_node.ExtractChildrenCntBefore(in_node_index);
  _file_manager.SetNode(curr_file_pos, curr_node);

  auto parent_node = Node<ElementType, T>();

  bool finished = false;
  while (!file_pos_path.empty() && !finished) {
    file_pos_t parent_file_pos = file_pos_path.back();
    unsigned in_parent_index = indexes_path.back();
    file_pos_path.pop_back();
    indexes_path.pop_back();
    finished = _CorrectNodeOnExtract(curr_node, parent_node,
                                     curr_file_pos, parent_file_pos,
                                     in_parent_index);
    curr_file_pos = parent_file_pos;
    curr_node = parent_node;
  }
  _CorrectChildrenCnts(file_pos_path, indexes_path, -1);
  Node<ElementType, T> root = _file_manager.GetNode(_data_info_ptr->_root_pos);
  if (root.Size() == 0) {
    _file_manager.DeleteNode(_data_info_ptr->_root_pos);
    _data_info_ptr->_root_pos = root.LinkBefore(0);
    root =  _file_manager.GetNode(_data_info_ptr->_root_pos);
    root.SetIsRoot(true);
    _file_manager.SetNode(_data_info_ptr->_root_pos, root);
  }
  return element_to_return;
}

/*
 * Corrects node after extracting element from it by exchanging elements and
 * links with neighbour and parent.
 * Requires root node not to be empty.
 * Nodes are expected to be opened.
 */

template <typename ElementType, size_t T>
bool BTreeList<ElementType, T>::_CorrectNodeOnExtract(
    Node<ElementType, T> &node,
    Node<ElementType, T> &parent_node,
    file_pos_t file_pos,
    file_pos_t parent_file_pos,
    unsigned in_parent_index
) {
  bool finished = false;
  parent_node = _file_manager.GetNode(parent_file_pos);

  if (node.Size() >= T - 1) {
    --parent_node.ChildrenCntBefore(in_parent_index);
    _file_manager.SetNode(parent_file_pos, parent_node);
    return true;
  }

  file_pos_t neighbour_node_file_pos;
  bool with_left = true;

  if (in_parent_index == 0) {
    neighbour_node_file_pos = parent_node.LinkAfter(in_parent_index);
    with_left = false;
  } else {  // in_parent_index > 0
    --in_parent_index;
    neighbour_node_file_pos = parent_node.LinkBefore(in_parent_index);
  }

  // in_parent_index is now an index of element between
  // node and neighbour_node in parent

  Node<ElementType, T> neighbour_node =
      _file_manager.GetNode(neighbour_node_file_pos);

  if (neighbour_node.Size() == T - 1) {  // connect
    Node<ElementType, T> connected_node;
    if (with_left) {
      connected_node =
          Connect(neighbour_node, node, parent_node.Extract(in_parent_index));
      parent_node.ExtractLinkBefore(in_parent_index);
      parent_node.ExtractChildrenCntBefore(in_parent_index);
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
    if (parent_node.Size() >= T - 1 || parent_node.GetIsRoot()) {
      _file_manager.SetNode(parent_file_pos, parent_node);
      finished = true;
    }
  } else {  // move element
    if (with_left) {
      _MoveElementFromLeftNeighbour(node, neighbour_node, parent_node, in_parent_index);
    } else {  // with right
      _MoveElementFromRightNeighbour(node, neighbour_node, parent_node, in_parent_index);
    }
    _file_manager.SetNode(parent_file_pos, parent_node);
    _file_manager.SetNode(file_pos, node);
    _file_manager.SetNode(neighbour_node_file_pos, neighbour_node);
    finished = true;  // After moving everything is always OK
  }
  return finished;
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_MoveElementFromLeftNeighbour(
    Node<ElementType, T> &node,
    Node<ElementType, T> &neighbour_node,
    Node<ElementType, T> &parent_node,
    unsigned &in_parent_index
) {
  ElementType element_from_neighbour = neighbour_node.ExtractBack();
  file_pos_t link_from_neighbour = neighbour_node.ExtractBackLink();
  size_t cc_from_neighbour = neighbour_node.ExtractBackChildrenCnt();
  ElementType element_from_parent =
      parent_node.Element(in_parent_index);
  node.Insert(0, element_from_parent);
  node.LinkAfter(0) = node.LinkBefore(0);
  node.LinkBefore(0) = link_from_neighbour;
  node.ChildrenCntAfter(0) = node.ChildrenCntBefore(0);
  node.ChildrenCntBefore(0) = cc_from_neighbour;

  parent_node.Element(in_parent_index) = element_from_neighbour;
  parent_node.SetChildrenCnts(in_parent_index,
                              neighbour_node.GetAllChildrenCnt(),
                              node.GetAllChildrenCnt());
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_MoveElementFromRightNeighbour(
    Node<ElementType, T> &node,
    Node<ElementType, T> &neighbour_node,
    Node<ElementType, T> &parent_node,
    unsigned &in_parent_index
) {
  ElementType element_from_neighbour = neighbour_node.Extract(0);
  file_pos_t link_from_neighbour = neighbour_node.ExtractLinkBefore(0);
  size_t cc_from_neighbour = neighbour_node.ExtractChildrenCntBefore(0);
  ElementType element_from_parent = parent_node.Element(in_parent_index);
  node.PushBack(element_from_parent);
  node.LinkAfter(node.Size() - 1) = link_from_neighbour;
  node.ChildrenCntAfter(node.Size() - 1) = cc_from_neighbour;

  parent_node.Element(in_parent_index) = element_from_neighbour;
  parent_node.SetChildrenCnts(in_parent_index,
                              node.GetAllChildrenCnt(),
                              neighbour_node.GetAllChildrenCnt());
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_CorrectChildrenCnts(
    std::vector<file_pos_t> &file_pos_path,
    std::vector<unsigned> &in_node_indexes_path,
    int to_change
) {
  while (!file_pos_path.empty()) {
    Node<ElementType, T> node_to_correct =
        _file_manager.GetNode(file_pos_path.back());
    node_to_correct.ChildrenCntBefore(in_node_indexes_path.back()) +=
        to_change;
    _file_manager.SetNode(file_pos_path.back(), node_to_correct);
    file_pos_path.pop_back();
    in_node_indexes_path.pop_back();
  }
}

template <typename ElementType, size_t T>
void BTreeList<ElementType, T>::_Rebuild() {
  std::string restored_name = _file_manager._file_params_ptr->path;
  std::shared_ptr<DataInfo> new_data_info_ptr(std::make_shared<DataInfo>());
  new_data_info_ptr->_root_pos = 0;
  new_data_info_ptr->_size = _data_info_ptr->_size;
  new_data_info_ptr->_stack_head_pos = -1;
  new_data_info_ptr->_free_tail_start = _data_info_ptr->_size;
  FileSavingManager<ElementType, T> new_file_manager("data_tmp", new_data_info_ptr);
  file_pos_t last_added_node_pos = 0;
  file_pos_t curr_pos = 0;
  new_file_manager.SetNode(curr_pos, _file_manager.GetNode(_data_info_ptr->_root_pos));
  do {
    Node<ElementType, T> curr_node = new_file_manager.GetNode(curr_pos);
    if (!curr_node.GetIsLeaf()) {
      for (unsigned i = 0; i < curr_node.Size() + 1; ++i) {
        Node<ElementType, T> node_to_copy =
            _file_manager.GetNode(curr_node.LinkBefore(i));
        file_pos_t pos_to_set_node = new_file_manager.NewNode(node_to_copy);
        ++last_added_node_pos;
        curr_node.LinkBefore(i) = pos_to_set_node;
      }
      new_file_manager.SetNode(curr_pos, curr_node);
    }
    ++curr_pos;
  } while (curr_pos < last_added_node_pos);
  _file_manager = new_file_manager;
  std::filesystem::remove(restored_name);
  _file_manager.RenameMappedFile(restored_name);
  _data_info_ptr = new_data_info_ptr;
}

template <typename ElementType, size_t T>
BTreeList<ElementType, T>::~BTreeList<ElementType, T>() {
  if (_rebuild_flag) {
    _Rebuild();
  }
}


#endif //B_TREE_LIST_LIBRARY_H