//
// Created by gogagum on 14.07.2020.
//

#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

#ifndef B_TREE_LIST_LIB__NODE_HPP_
#define B_TREE_LIST_LIB__NODE_HPP_

typedef uint64_t file_pos_t;
typedef int64_t signed_file_pos_t;

////////////////////////////////////////////////////////////////////////////////
// Node                                                                       //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
class _Node{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _Node();

  _Node(const std::vector<_ElementType> &v,
        const std::vector<file_pos_t> &links,
        const std::vector<size_t>  &children_cnts,
        uint32_t flags = 0);

  _Node(std::vector<_ElementType> &&v,
        std::vector<file_pos_t> &&links,
        std::vector<size_t>  &&children_cnts,
        uint32_t flags = 0);

  _Node(const _Node<_ElementType, T> &other);

  _Node(_Node<_ElementType, T> &&other);

  //////////////////////////////////////////////////////////////////////////////

  _Node<_ElementType, T>& operator=(_Node<_ElementType, T> &&other);

  _Node<_ElementType, T>& operator=(const _Node<_ElementType, T> &other);

  //////////////////////////////////////////////////////////////////////////////

  void PushBack(const _ElementType& e);

  //////////////////////////////////////////////////////////////////////////////

  _ElementType& Element(unsigned i);

  file_pos_t& LinkAfter(unsigned i);

  file_pos_t& LinkBefore(unsigned i);

  void SetLinks(unsigned i, file_pos_t link_before, file_pos_t link_after);

  void SetChildrenCnts(unsigned i, size_t cc_before, size_t cc_after);

  size_t& ChildrenCntAfter(unsigned i);

  size_t& ChildrenCntBefore(unsigned i);

  size_t ChildrenCntAfter(unsigned i) const;

  size_t ChildrenCntBefore(unsigned i) const;

  //////////////////////////////////////////////////////////////////////////////

  void Insert(unsigned i, const _ElementType &e);

  //////////////////////////////////////////////////////////////////////////////

  _ElementType Extract(unsigned i);

  file_pos_t ExtractLinkAfter(unsigned i);

  file_pos_t ExtractLinkBefore(unsigned i);

  size_t ExtractChildrenCntAfter(unsigned i);

  size_t ExtractChildrenCntBefore(unsigned i);

  //////////////////////////////////////////////////////////////////////////////

  file_pos_t ExtractBackLink();

  size_t ExtractBackChildrenCnt();

  _ElementType ExtractBack();

  //////////////////////////////////////////////////////////////////////////////

  _Node<_ElementType, T> NodeFromFirstHalf();

  _Node<_ElementType, T> NodeFromSecondHalf();

  _ElementType GetMiddleElement();

  //////////////////////////////////////////////////////////////////////////////

  void ConnectWith(_ElementType e, const _Node<_ElementType, T> &other);

  //////////////////////////////////////////////////////////////////////////////

  [[nodiscard]] bool GetIsRoot() const;

  void SetIsRoot(bool flag_to_set);

  [[nodiscard]] bool GetIsLeaf() const;

  void SetIsLeaf(bool flag_to_set);

  [[nodiscard]] bool GetIsRightestLeaf() const;

  void SetIsRightestLeaf(bool flag_to_set);

  //////////////////////////////////////////////////////////////////////////////

  size_t GetAllChildrenCnt();

  //////////////////////////////////////////////////////////////////////////////

  [[nodiscard]] size_t Size() const;

  void Resize(size_t new_size);

  //////////////////////////////////////////////////////////////////////////////

  struct _NodeInfo;

  _NodeInfo GetNodeInfo() const;

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
    RIGHTEST = 4
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  std::vector<_ElementType> _elements;
  std::vector<file_pos_t> _links;
  std::vector<size_t> _children_cnts;
  uint32_t _flags;

  //////////////////////////////////////////////////////////////////////////////
  // Static fields                                                            //
  //////////////////////////////////////////////////////////////////////////////

  const static ptrdiff_t elements_offset = sizeof(struct _NodeInfo);
  const static ptrdiff_t links_offset =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(_ElementType);
  const static ptrdiff_t cc_offset =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(_ElementType) +
      (2 * T) * sizeof(file_pos_t);
  const static size_t inmemory_size =
      sizeof(struct _NodeInfo) + (2 * T - 1) * sizeof(_ElementType) +
      (2 * T) * (sizeof(file_pos_t) + sizeof(size_t));

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename ElementType, size_t _T>
  friend class BTreeList;

  template <typename ElementType, size_t _T>
  friend class _FileSavingManager;

  friend class _BlockRW;

  template <typename ElementType, size_t _T>
  friend _Node<ElementType, _T> Connect(const _Node<ElementType, _T> &left_node,
                                        const _Node<ElementType, _T> &right_node,
                                        const ElementType &element);
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_Node<_ElementType, T>::_Node()
  : _elements(std::vector<_ElementType>(0)),
    _links(std::vector<file_pos_t>(1, 0)),
    _children_cnts(std::vector<size_t>(1, 0)),
    _flags(0) {}

template <typename _ElementType, size_t T>
_Node<_ElementType, T>::_Node(
    const std::vector<_ElementType>& v,
    const std::vector<file_pos_t> &links,
    const std::vector<uint64_t> &children_cnts,
    uint32_t flags
) : _elements(v),
    _links(links),
    _children_cnts(children_cnts),
    _flags(flags) {}

template <typename _ElementType, size_t T>
_Node<_ElementType, T>::_Node(
    std::vector<_ElementType> &&v,
    std::vector<file_pos_t> &&links,
    std::vector<uint64_t> &&children_cnts,
    uint32_t flags
) : _elements(std::move(v)),
    _links(std::move(links)),
    _children_cnts(std::move(children_cnts)),
    _flags(flags) {}

template <typename _ElementType, size_t T>
_Node<_ElementType, T>::_Node(const _Node<_ElementType, T> &other)
  : _elements(other._elements),
    _links(other._links),
    _children_cnts(other._children_cnts),
    _flags(other._flags) {}

template <typename _ElementType, size_t T>
_Node<_ElementType, T>::_Node(_Node<_ElementType, T> &&other)
  : _elements(std::move(other._elements)),
    _links(std::move(other._links)),
    _children_cnts(std::move(other._children_cnts)),
    _flags(other._flags) {}

////////////////////////////////////////////////////////////////////////////////
// Assignment operator                                                        //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_Node<_ElementType, T>& _Node<_ElementType, T>::operator=(
    _Node<_ElementType, T> &&other
) {
  _elements = std::move(other._elements);
  _links = std::move(other._links);
  _children_cnts = std::move(other._children_cnts);
  _flags = other._flags;
  return *this;
}

template <typename _ElementType, size_t T>
_Node<_ElementType, T>& _Node<_ElementType, T>::operator=(
    const _Node<_ElementType, T> &other
) {
  _elements = other._elements;
  _links = other._links;
  _children_cnts = other._children_cnts;
  _flags = other._flags;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
// Getters                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_ElementType& _Node<_ElementType, T>::Element(unsigned i) {
  return _elements[i];
}

template <typename _ElementType, size_t T>
file_pos_t& _Node<_ElementType, T>::LinkAfter(unsigned i) {
  return _links[i + 1];
}

template <typename _ElementType, size_t T>
file_pos_t& _Node<_ElementType, T>::LinkBefore(unsigned i) {
  return _links[i];
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::SetLinks(
    unsigned i,
    file_pos_t link_before,
    file_pos_t link_after
) {
  LinkBefore(i) = link_before;
  LinkAfter(i) = link_after;
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::SetChildrenCnts(
    unsigned i,
    size_t cc_before,
    size_t cc_after
) {
  ChildrenCntBefore(i) = cc_before;
  ChildrenCntAfter(i) = cc_after;
}

template <typename _ElementType, size_t T>
size_t& _Node<_ElementType, T>::ChildrenCntAfter(unsigned i) {
  return _children_cnts[i + 1];
}

template <typename _ElementType, size_t T>
size_t& _Node<_ElementType, T>::ChildrenCntBefore(unsigned i) {
  return _children_cnts[i];
}

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::ChildrenCntAfter(unsigned i) const {
  return _children_cnts[i + 1];
}

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::ChildrenCntBefore(unsigned i) const {
  return _children_cnts[i];
}

////////////////////////////////////////////////////////////////////////////////
// Push backs                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::PushBack(const _ElementType &e) {
  _elements.push_back(e);
  _links.push_back(0);
  _children_cnts.push_back(0);
}

////////////////////////////////////////////////////////////////////////////////
// Inserts                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::Insert(unsigned i, const _ElementType &e) {
  _elements.insert(_elements.begin() + i, e);
  _links.insert(_links.begin() + i + 1, 0);
  _children_cnts.insert(_children_cnts.begin() + i + 1, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Extracts                                                                   //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_ElementType _Node<_ElementType, T>::Extract(unsigned i) {
  _ElementType element = _elements[i];
  _elements.erase(_elements.begin() + i);
  return element;
}

template <typename _ElementType, size_t T>
file_pos_t _Node<_ElementType, T>::ExtractLinkAfter(unsigned i) {
  unsigned index = _links[i + 1];
  _links.erase(_links.begin() + i + 1);
  return index;
}

template <typename _ElementType, size_t T>
file_pos_t _Node<_ElementType, T>::ExtractLinkBefore(unsigned i) {
  unsigned index = _links[i];
  _links.erase(_links.begin() + i);
  return index;
}

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::ExtractChildrenCntAfter(unsigned i) {
  size_t cnt = _children_cnts[i + 1];
  _children_cnts.erase(_children_cnts.begin() + i + 1);
  return cnt;
}

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::ExtractChildrenCntBefore(unsigned i) {
  size_t cnt = _children_cnts[i];
  _children_cnts.erase(_children_cnts.begin() + i);
  return cnt;
}

////////////////////////////////////////////////////////////////////////////////
// Extract back                                                               //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
file_pos_t _Node<_ElementType, T>::ExtractBackLink() {
  file_pos_t index = _links.back();
  _links.pop_back();
  return index;
}

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::ExtractBackChildrenCnt() {
  size_t cnt = _children_cnts.back();
  _children_cnts.pop_back();
  return cnt;
}

template <typename _ElementType, size_t T>
_ElementType _Node<_ElementType, T>::ExtractBack() {
  _ElementType element = _elements.back();
  _elements.pop_back();
  return element;
}

////////////////////////////////////////////////////////////////////////////////
// Separators                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
_Node<_ElementType, T> _Node<_ElementType, T>::NodeFromFirstHalf() {
  return _Node<_ElementType, T> (
    std::vector<_ElementType>(_elements.begin(),
                              _elements.begin() + _elements.size() / 2),
    std::vector<file_pos_t>(_links.begin(),
                            _links.begin() + _links.size() / 2),
    std::vector<size_t>(_children_cnts.begin(),
                        _children_cnts.begin() + _children_cnts.size() / 2),
    this->_flags & ~_Flags::RIGHTEST & ~_Flags::ROOT
  );
}

template <typename _ElementType, size_t T>
_Node<_ElementType, T> _Node<_ElementType, T>::NodeFromSecondHalf() {
  return _Node<_ElementType, T>(
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
_ElementType _Node<_ElementType, T>::GetMiddleElement() {
  return *(_elements.begin() + _elements.size() / 2);
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::ConnectWith(_ElementType e,
                                         const _Node<_ElementType, T> &other) {
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

template <typename _ElementType, size_t T>
bool _Node<_ElementType, T>::GetIsRoot() const {
  return _flags & _Flags::ROOT ;
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::SetIsRoot(bool flag_to_set) {
  _flags = (_flags & ~1UL) | flag_to_set;
}

template <typename _ElementType, size_t T>
bool _Node<_ElementType, T>::GetIsLeaf() const {
  return _flags & _Flags::LEAF;
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::SetIsLeaf(bool flag_to_set) {
  _flags = (_flags & ~2UL) | (flag_to_set << 1);
}

template <typename _ElementType, size_t T>
bool _Node<_ElementType, T>::GetIsRightestLeaf() const {
  return _flags & _Flags::RIGHTEST;
}

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::SetIsRightestLeaf(bool flag_to_set) {
  _flags = (_flags & ~4UL) | (flag_to_set << 2);
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::GetAllChildrenCnt() {
  size_t sum = 0;
  for (auto i: _children_cnts) {
    sum += i;
  }
  sum += _elements.size();
  return sum;
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
size_t _Node<_ElementType, T>::Size() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
void _Node<_ElementType, T>::Resize(size_t new_size) {
  _elements.resize(new_size);
  _links.resize(new_size + 1);
  _children_cnts.resize(new_size + 1);
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType, size_t T>
struct _Node<_ElementType, T>::_NodeInfo
_Node<_ElementType, T>::GetNodeInfo() const {
  return _Node<_ElementType, T>::_NodeInfo{_elements.size(), _flags};
}

////////////////////////////////////////////////////////////////////////////////
// Friend functions                                                           //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType, size_t T>
_Node<ElementType, T> Connect(const _Node<ElementType, T> &left_node,
                              const _Node<ElementType, T> &right_node,
                              const ElementType &e) {
  _Node<ElementType, T> node_to_return = left_node;
  node_to_return.ConnectWith(e, right_node);
  return node_to_return;
};


#endif //B_TREE_LIST_LIB__NODE_HPP_
