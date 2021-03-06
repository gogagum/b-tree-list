//
// Created by gogagum on 14.07.2020.
//

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>
#include <boost/interprocess/mapped_region.hpp>

#ifndef B_TREE_LIST_LIB__NODE_HPP_
#define B_TREE_LIST_LIB__NODE_HPP_

typedef uint64_t file_pos_t;
typedef int64_t signed_file_pos_t;

inline int CeilDiv(int a, int b) {
  return (a - 1) / b + 1;
}

////////////////////////////////////////////////////////////////////////////////
// Node                                                                       //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
class Node{
 private:
  struct _NodeInfo;

  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  Node();

  Node(const std::vector<ElementType> &v,
       const std::vector<file_pos_t> &links,
       const std::vector<size_t>  &children_cnts,
       uint32_t flags = 0);

  Node(std::vector<ElementType> &&v,
       std::vector<file_pos_t> &&links,
       std::vector<size_t>  &&children_cnts,
       uint32_t flags = 0);

  Node(const Node<ElementType, T> &other);

  Node(Node<ElementType, T> &&other);

  //////////////////////////////////////////////////////////////////////////////
  // Assign operator                                                          //
  //////////////////////////////////////////////////////////////////////////////

  Node<ElementType, T>& operator=(Node<ElementType, T> &&other);

  Node<ElementType, T>& operator=(const Node<ElementType, T> &other);

  //////////////////////////////////////////////////////////////////////////////
  // Getters/setters                                                          //
  //////////////////////////////////////////////////////////////////////////////

  ElementType& Element(unsigned i);

  file_pos_t& LinkAfter(unsigned i);

  file_pos_t& LinkBefore(unsigned i);

  [[maybe_unused]] file_pos_t LinkAfter(unsigned i) const;

  file_pos_t LinkBefore(unsigned i) const;

  size_t& ChildrenCntAfter(unsigned i);

  size_t& ChildrenCntBefore(unsigned i);

  [[maybe_unused]] size_t ChildrenCntAfter(unsigned i) const;

  [[maybe_unused]] size_t ChildrenCntBefore(unsigned i) const;

  void SetLinks(unsigned i, file_pos_t link_before, file_pos_t link_after);

  void SetChildrenCnts(unsigned i, size_t cc_before, size_t cc_after);

  _NodeInfo GetNodeInfo() const;

  //////////////////////////////////////////////////////////////////////////////
  // Adding elements methods                                                  //
  //////////////////////////////////////////////////////////////////////////////

  void PushBack(const ElementType& e);

  void Insert(unsigned i, const ElementType &e);

  template <typename IteratorType>
  void Insert(unsigned i, const IteratorType &begin, const IteratorType &end);

  //////////////////////////////////////////////////////////////////////////////
  // Extract methods                                                          //
  //////////////////////////////////////////////////////////////////////////////

  ElementType Extract(unsigned i);

  file_pos_t ExtractLinkAfter(unsigned i);

  file_pos_t ExtractLinkBefore(unsigned i);

  size_t ExtractChildrenCntAfter(unsigned i);

  size_t ExtractChildrenCntBefore(unsigned i);

  file_pos_t ExtractBackLink();

  size_t ExtractBackChildrenCnt();

  ElementType ExtractBack();

  //////////////////////////////////////////////////////////////////////////////
  // Separation functions                                                     //
  //////////////////////////////////////////////////////////////////////////////

  Node<ElementType, T> NodeFromFirstHalf();

  Node<ElementType, T> NodeFromSecondHalf();

  ElementType GetMiddleElement() const;

  //////////////////////////////////////////////////////////////////////////////
  // Connect                                                                  //
  //////////////////////////////////////////////////////////////////////////////

  void ConnectWith(ElementType e, const Node<ElementType, T> &other);

  //////////////////////////////////////////////////////////////////////////////
  // Flags setters/getters                                                    //
  //////////////////////////////////////////////////////////////////////////////

  [[nodiscard]] bool GetIsRoot() const;

  void SetIsRoot(bool flag_to_set);

  [[nodiscard]] bool GetIsLeaf() const;

  [[maybe_unused]] void SetIsLeaf(bool flag_to_set);

  //////////////////////////////////////////////////////////////////////////////
  // Size getters                                                             //
  //////////////////////////////////////////////////////////////////////////////

  [[nodiscard]] size_t GetAllChildrenCnt() const;

  [[nodiscard]] size_t Size() const;

  [[nodiscard]] size_t ElementsArraySize() const;

  [[nodiscard]] size_t LinksArraySize() const;

  [[nodiscard]] size_t CCArraySize() const;

  //////////////////////////////////////////////////////////////////////////////
  // Resize functions                                                         //
  //////////////////////////////////////////////////////////////////////////////

  void Resize(size_t new_size);

  void Resize(size_t new_size, const ElementType& element_type);

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs and enums declaration                                 //
  //////////////////////////////////////////////////////////////////////////////

  struct _NodeInfo{
    size_t _elements_cnt;
    uint32_t _flags;
  };

  enum _Flags{
    ROOT = 1,
    LEAF = 2,
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::vector<ElementType> _elements;
  std::vector<file_pos_t> _links;
  std::vector<size_t> _children_cnts;
  uint32_t _flags;

  //////////////////////////////////////////////////////////////////////////////
  // Static fields                                                            //
  //////////////////////////////////////////////////////////////////////////////

  const static size_t info_size = sizeof(struct _NodeInfo);
  const static ptrdiff_t elements_offset = sizeof(struct _NodeInfo);
  const static ptrdiff_t links_offset =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(ElementType);
  const static ptrdiff_t cc_offset =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(ElementType) +
      (2 * T) * sizeof(file_pos_t);
  const static size_t inmemory_size =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(ElementType) +
      (2 * T) * (sizeof(file_pos_t) + sizeof(size_t));

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename _ElementType, size_t _T>
  friend class BTreeList;

  template <typename _ElementType, size_t _T>
  friend class FileSavingManager;

  friend class BlockRW;

  template <typename _ElementType, size_t _T>
  friend Node<_ElementType, _T> Connect(const Node<_ElementType, _T> &left_node,
                                        const Node<_ElementType, _T> &right_node,
                                        const _ElementType &element);
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
Node<_ElementType, T>::Node()
  : _elements(0),
    _links(1, static_cast<file_pos_t>(0)),
    _children_cnts(1, static_cast<size_t>(0)),
    _flags(_Flags::ROOT | _Flags::LEAF) {}

template <typename _ElementType, size_t T>
Node<_ElementType, T>::Node(
    const std::vector<_ElementType>& v,
    const std::vector<file_pos_t> &links,
    const std::vector<uint64_t> &children_cnts,
    uint32_t flags
) : _elements(v),
    _links(links),
    _children_cnts(children_cnts),
    _flags(flags) {}

template <typename _ElementType, size_t T>
Node<_ElementType, T>::Node(
    std::vector<_ElementType> &&v,
    std::vector<file_pos_t> &&links,
    std::vector<uint64_t> &&children_cnts,
    uint32_t flags
) : _elements(std::move(v)),
    _links(std::move(links)),
    _children_cnts(std::move(children_cnts)),
    _flags(flags) {}

template <typename _ElementType, size_t T>
Node<_ElementType, T>::Node(const Node<_ElementType, T> &other)
  : _elements(other._elements),
    _links(other._links),
    _children_cnts(other._children_cnts),
    _flags(other._flags) {}

template <typename _ElementType, size_t T>
Node<_ElementType, T>::Node(Node<_ElementType, T> &&other)
  : _elements(std::move(other._elements)),
    _links(std::move(other._links)),
    _children_cnts(std::move(other._children_cnts)),
    _flags(other._flags) {}

////////////////////////////////////////////////////////////////////////////////
// Assignment operator                                                        //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
Node<_ElementType, T>& Node<_ElementType, T>::operator=(
    Node<_ElementType, T> &&other
) {
  _elements = std::move(other._elements);
  _links = std::move(other._links);
  _children_cnts = std::move(other._children_cnts);
  _flags = other._flags;
  return *this;
}

template <typename _ElementType, size_t T>
Node<_ElementType, T>& Node<_ElementType, T>::operator=(
    const Node<_ElementType, T> &other
) {
  _elements = other._elements;
  _links = other._links;
  _children_cnts = other._children_cnts;
  _flags = other._flags;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Getters/setters                                                            //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_ElementType& Node<_ElementType, T>::Element(unsigned i) {
  return _elements[i];
}

template <typename _ElementType, size_t T>
file_pos_t& Node<_ElementType, T>::LinkAfter(unsigned i) {
  return _links[i + 1];
}

template <typename _ElementType, size_t T>
file_pos_t& Node<_ElementType, T>::LinkBefore(unsigned i) {
  return _links[i];
}

template <typename _ElementType, size_t T>
[[maybe_unused]] file_pos_t Node<_ElementType, T>::LinkAfter(unsigned i) const {
  return _links[i + 1];
}

template <typename _ElementType, size_t T>
file_pos_t Node<_ElementType, T>::LinkBefore(unsigned i) const {
  return _links[i];
}

template <typename _ElementType, size_t T>
size_t& Node<_ElementType, T>::ChildrenCntAfter(unsigned i) {
  return _children_cnts[i + 1];
}

template <typename _ElementType, size_t T>
size_t& Node<_ElementType, T>::ChildrenCntBefore(unsigned i) {
  return _children_cnts[i];
}

template <typename _ElementType, size_t T>
[[maybe_unused]] size_t Node<_ElementType, T>::ChildrenCntAfter(unsigned i) const {
  return _children_cnts[i + 1];
}

template <typename _ElementType, size_t T>
[[maybe_unused]] size_t Node<_ElementType, T>::ChildrenCntBefore(unsigned i) const {
  return _children_cnts[i];
}

template <typename _ElementType, size_t T>
void Node<_ElementType, T>::SetLinks(
    unsigned i,
    file_pos_t link_before,
    file_pos_t link_after
) {
  LinkBefore(i) = link_before;
  LinkAfter(i) = link_after;
}

template <typename _ElementType, size_t T>
void Node<_ElementType, T>::SetChildrenCnts(
    unsigned i,
    size_t cc_before,
    size_t cc_after
) {
  ChildrenCntBefore(i) = cc_before;
  ChildrenCntAfter(i) = cc_after;
}

template <typename ElementType, size_t T>
struct Node<ElementType, T>::_NodeInfo
Node<ElementType, T>::GetNodeInfo() const {
  return Node<ElementType, T>::_NodeInfo{Size(), _flags};
}

////////////////////////////////////////////////////////////////////////////////
// Adding elements                                                            //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
void Node<_ElementType, T>::PushBack(const _ElementType &e) {
  _elements.push_back(e);
  _links.push_back(0);
  _children_cnts.push_back(0);
}

template <typename _ElementType, size_t T>
void Node<_ElementType, T>::Insert(unsigned i, const _ElementType &e) {
  _elements.insert(_elements.begin() + i, e);
  _links.insert(_links.begin() + i + 1, 0);
  _children_cnts.insert(_children_cnts.begin() + i + 1, 0);
}

template<typename ElementType, size_t T>
template<typename IteratorType>
void Node<ElementType, T>::Insert(unsigned int i,
                                  const IteratorType &begin,
                                  const IteratorType &end) {
  int cnt = end - begin;
  _elements.insert(_elements.begin() + i, begin, end);
  std::vector<file_pos_t> links_to_insert(cnt, 0);
  _links.insert(_links.begin() + i + 1,
                links_to_insert.begin(),
                links_to_insert.end());
  std::vector<size_t> cnts_to_insert(cnt, 0);
  _children_cnts.insert(_children_cnts.begin() + i + 1,
                        cnts_to_insert.begin(),
                        cnts_to_insert.end());
}

////////////////////////////////////////////////////////////////////////////////
// Extracts                                                                   //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_ElementType Node<_ElementType, T>::Extract(unsigned i) {
  _ElementType element = _elements[i];
  _elements.erase(_elements.begin() + i);
  return element;
}

template <typename _ElementType, size_t T>
file_pos_t Node<_ElementType, T>::ExtractLinkAfter(unsigned i) {
  unsigned index = _links[i + 1];
  _links.erase(_links.begin() + i + 1);
  return index;
}

template <typename _ElementType, size_t T>
file_pos_t Node<_ElementType, T>::ExtractLinkBefore(unsigned i) {
  unsigned index = _links[i];
  _links.erase(_links.begin() + i);
  return index;
}

template <typename _ElementType, size_t T>
size_t Node<_ElementType, T>::ExtractChildrenCntAfter(unsigned i) {
  size_t cnt = _children_cnts[i + 1];
  _children_cnts.erase(_children_cnts.begin() + i + 1);
  return cnt;
}

template <typename _ElementType, size_t T>
size_t Node<_ElementType, T>::ExtractChildrenCntBefore(unsigned i) {
  size_t cnt = _children_cnts[i];
  _children_cnts.erase(_children_cnts.begin() + i);
  return cnt;
}

template <typename _ElementType, size_t T>
file_pos_t Node<_ElementType, T>::ExtractBackLink() {
  file_pos_t index = _links.back();
  _links.pop_back();
  return index;
}

template <typename _ElementType, size_t T>
size_t Node<_ElementType, T>::ExtractBackChildrenCnt() {
  size_t cnt = _children_cnts.back();
  _children_cnts.pop_back();
  return cnt;
}

template <typename _ElementType, size_t T>
_ElementType Node<_ElementType, T>::ExtractBack() {
  _ElementType element = _elements.back();
  _elements.pop_back();
  return element;
}

////////////////////////////////////////////////////////////////////////////////
// Separation functions                                                       //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
Node<_ElementType, T> Node<_ElementType, T>::NodeFromFirstHalf() {
  return Node<_ElementType, T> (
    std::vector<_ElementType>(_elements.begin(),
                              _elements.begin() + _elements.size() / 2),
    std::vector<file_pos_t>(_links.begin(),
                            _links.begin() + _links.size() / 2),
    std::vector<size_t>(_children_cnts.begin(),
                        _children_cnts.begin() + _children_cnts.size() / 2),
    this->_flags & ~_Flags::ROOT
  );
}

template <typename _ElementType, size_t T>
Node<_ElementType, T> Node<_ElementType, T>::NodeFromSecondHalf() {
  return Node<_ElementType, T>(
    std::vector<_ElementType>(_elements.end() - _elements.size() / 2,
                              _elements.end()),
    std::vector<file_pos_t>(_links.end() - _links.size() / 2,
                            _links.end()),
    std::vector<size_t>(_children_cnts.end() - _children_cnts.size() / 2,
                        _children_cnts.end()),
    this->_flags & ~_Flags::ROOT
  );
}

template <typename _ElementType, size_t T>
_ElementType Node<_ElementType, T>::GetMiddleElement() const {
  return *(_elements.begin() + _elements.size() / 2);
}

////////////////////////////////////////////////////////////////////////////////
// Connect                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
void Node<_ElementType, T>::ConnectWith(_ElementType e,
                                        const Node<_ElementType, T> &other) {
  _elements.push_back(e);
  _elements.insert(_elements.end(),
                   other._elements.begin(),
                   other._elements.end());
  _links.insert(_links.end(), other._links.begin(), other._links.end());
  _children_cnts.insert(_children_cnts.end(),
                        other._children_cnts.begin(),
                        other._children_cnts.end());
}

////////////////////////////////////////////////////////////////////////////////
// Flags setters and getters                                                  //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
bool Node<ElementType, T>::GetIsRoot() const {
  return _flags & _Flags::ROOT ;
}

template <typename ElementType, size_t T>
void Node<ElementType, T>::SetIsRoot(bool flag_to_set) {
  _flags = (_flags & ~1UL) | flag_to_set;
}

template <typename ElementType, size_t T>
bool Node<ElementType, T>::GetIsLeaf() const {
  return _flags & _Flags::LEAF;
}

template <typename ElementType, size_t T>
[[maybe_unused]] void Node<ElementType, T>::SetIsLeaf(bool flag_to_set) {
  _flags = (_flags & ~2UL) | (flag_to_set << 1);
}

////////////////////////////////////////////////////////////////////////////////
// Size getters                                                               //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
size_t Node<ElementType, T>::GetAllChildrenCnt() const {
  size_t sum = 0;
  for (auto i: _children_cnts) {
    sum += i;
  }
  sum += _elements.size();
  return sum;
}

template <typename ElementType, size_t T>
size_t Node<ElementType, T>::Size() const {
  return _elements.size();
}

template <typename ElementType, size_t T>
size_t Node<ElementType, T>::ElementsArraySize() const {
  return _elements.size() * sizeof(ElementType);
}

template <typename ElementType, size_t T>
size_t Node<ElementType, T>::LinksArraySize() const {
  return _links.size() * sizeof(file_pos_t);
}

template <typename ElementType, size_t T>
size_t Node<ElementType, T>::CCArraySize() const {
  return _children_cnts.size() * sizeof(size_t);
}

////////////////////////////////////////////////////////////////////////////////
// Resize functions                                                           //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
void Node<ElementType, T>::Resize(size_t new_size) {
  _elements.resize(new_size);
  _links.resize(new_size + 1, static_cast<file_pos_t >(0));
  _children_cnts.resize(new_size + 1, static_cast<size_t >(0));
}

template <typename ElementType, size_t T>
void Node<ElementType, T>::Resize(size_t new_size,
                                  const ElementType& element_to_fill) {
  _elements.resize(new_size, element_to_fill);
  _links.resize(new_size + 1, static_cast<file_pos_t >(0));
  _children_cnts.resize(new_size + 1, static_cast<size_t >(0));
}

////////////////////////////////////////////////////////////////////////////////
// Friend functions                                                           //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
Node<ElementType, T> Connect(const Node<ElementType, T> &left_node,
                             const Node<ElementType, T> &right_node,
                             const ElementType &e) {
  Node<ElementType, T> node_to_return = left_node;
  node_to_return.ConnectWith(e, right_node);
  return node_to_return;
}

#endif //B_TREE_LIST_LIB__NODE_HPP_
