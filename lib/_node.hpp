//
// Created by gogagum on 14.07.2020.
//

#ifndef B_TREE_LIST_LIB__NODE_HPP_
#define B_TREE_LIST_LIB__NODE_HPP_

////////////////////////////////////////////////////////////////////////////////
// Node                                                                       //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
class _Node{
 private:
  //////////////////////////////////////////////////////////////////////////////
  // Functions                                                                //
  //////////////////////////////////////////////////////////////////////////////

  _Node();

  _Node(const std::vector<_ElementType>& v,
        const std::vector<unsigned int>& links,
        const std::vector<unsigned int>& children_cnts,
        uint32_t flags = 0);

  //////////////////////////////////////////////////////////////////////////////

  void Set(unsigned int i, const _ElementType &e);

  //void SetLink(unsigned int i, unsigned int l);

  //void SetChildrenCnt(unsigned int i, unsigned int cc);

  void SetLinkAfter(unsigned int i, unsigned int l);

  void SetLinkBefore(unsigned int i, unsigned int l);

  void SetChildrenCntAfter(unsigned int i, unsigned int cc);

  void SetChildrenCntBefore(unsigned int i, unsigned int cc);

  //////////////////////////////////////////////////////////////////////////////

  void PushBack(const _ElementType& e);

  void PushBackLink(unsigned int l);

  void PushBackChildrenCnt(unsigned int cc);

  void PushBackTrio(const _ElementType& e, unsigned int l, unsigned int cc);

  //////////////////////////////////////////////////////////////////////////////

  _ElementType Get(unsigned int i) const;

  unsigned int GetLinkAfter(unsigned int i) const;

  unsigned int GetLinkBefore(unsigned int i) const;

  unsigned int GetChildrenCntAfter(unsigned int i) const;

  unsigned int GetChildrenCntBefore(unsigned int i) const;

  //////////////////////////////////////////////////////////////////////////////

  void Insert(unsigned int i, const _ElementType &e);

  void InsertLink(unsigned int i, unsigned int l);

  void InsertChildrenCnt(unsigned int i, unsigned int cc);

  void InsertTrio(unsigned int i, const _ElementType &e,
                  unsigned int l, unsigned int cc);

  //////////////////////////////////////////////////////////////////////////////

  _ElementType Extract(unsigned int i);

  unsigned int ExtractLinkAfter(unsigned int i);

  unsigned int ExtractLinkBefore(unsigned int i);

  unsigned int ExtractChildrenCntAfter(unsigned int i);

  unsigned int ExtractChildrenCntBefore(unsigned int i);

  //////////////////////////////////////////////////////////////////////////////

  unsigned int ExtractBackChildrenCnt();

  unsigned int ExtractBackLink();

  _ElementType ExtractBack();

  //////////////////////////////////////////////////////////////////////////////

  _Node<_ElementType> NodeFromFirstHalf();

  _Node<_ElementType> NodeFromSecondHalf();

  _ElementType GetMiddleElement();

  //////////////////////////////////////////////////////////////////////////////

  void ConnectWith(_ElementType e, const _Node<_ElementType> &other);

  //////////////////////////////////////////////////////////////////////////////

  bool GetIsRoot() const;

  void SetIsRoot(bool flag_to_set);

  bool GetIsLeaf() const;

  void SetIsLeaf(bool flag_to_set);

  bool GetIsRightestLeaf() const;

  void SetIsRightestLeaf(bool flag_to_set);

  //////////////////////////////////////////////////////////////////////////////

  unsigned int GetAllChildrenCnt();

  //////////////////////////////////////////////////////////////////////////////

  size_t Size() const;

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
    RIGHTESTS = 4
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////


  _NodeInfo _info;
  std::vector<_ElementType> _elements;
  std::vector<unsigned int> _links;
  std::vector<unsigned int> _children_cnts;

  //////////////////////////////////////////////////////////////////////////////
  // Friend classes                                                           //
  //////////////////////////////////////////////////////////////////////////////

  template <typename ElementType>
  friend class BTreeList;

  template <typename ElementType>
  friend class _FileSavingManager;

  template <typename ElementType>
  friend _Node<ElementType> Connect(const _Node<ElementType> &left_node,
                                    const _Node<ElementType> &right_node,
                                    const ElementType element);
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_Node<_ElementType>::_Node() : _elements(std::vector<_ElementType>(0)) {
  _info._elements_cnt = 0;
  _info._flags = 0;
}

template <typename _ElementType>
_Node<_ElementType>::_Node(
    const std::vector<_ElementType>& v,
    const std::vector<unsigned int>& links,
    const std::vector<unsigned int>& children_cnts,
    uint32_t flags)
  : _elements(v),
    _links(links),
    _children_cnts(children_cnts) {
  _info._elements_cnt = _elements.size();
  _info._flags = flags;
}

////////////////////////////////////////////////////////////////////////////////
// Setters                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::Set(unsigned int i, const _ElementType &e) {
  _elements[i] = e;
}

template <typename _ElementType>
void _Node<_ElementType>::SetLinkAfter(unsigned int i, unsigned int l) {
  _links[i + 1] = l;
}

template <typename _ElementType>
void _Node<_ElementType>::SetLinkBefore(unsigned int i, unsigned int l) {
  _links[i] = l;
}

template <typename _ElementType>
void _Node<_ElementType>::SetChildrenCntAfter(unsigned int i, unsigned int cc) {
  _children_cnts[i + 1] = cc;
}

template <typename _ElementType>
void _Node<_ElementType>::SetChildrenCntBefore(unsigned int i,
                                               unsigned int cc) {
  _children_cnts[i] = cc;
}

////////////////////////////////////////////////////////////////////////////////
// Getters                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_ElementType _Node<_ElementType>::Get(unsigned int i) const {
  return _elements[i];
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetLinkAfter(unsigned int i) const {
  return _links[i + 1];
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetLinkBefore(unsigned int i) const {
  return _links[i];
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetChildrenCntAfter(unsigned int i) const {
  return _children_cnts[i + 1];
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetChildrenCntBefore(unsigned int i) const {
  return _children_cnts[i];
}

////////////////////////////////////////////////////////////////////////////////
// Push backs                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::PushBack(const _ElementType &e) {
  _elements.push_back(e);
  ++_info._elements_cnt;
}

template <typename _ElementType>
void _Node<_ElementType>::PushBackLink(unsigned int l) {
  _links.push_back(l);
}

template <typename _ElementType>
void _Node<_ElementType>::PushBackChildrenCnt(unsigned int cc) {
  _children_cnts.push_back(cc);
}

template <typename _ElementType>
void _Node<_ElementType>::PushBackTrio(const _ElementType &e,
                                       unsigned int l,
                                       unsigned int cc) {
  PushBack(e);
  PushBackLink(l);
  PushBackChildrenCnt(cc);
}

////////////////////////////////////////////////////////////////////////////////
// Inserts                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::Insert(unsigned int i, const _ElementType &e) {
  _elements.insert(_elements.begin() + i, e);
  ++_info._elements_cnt;
}

template <typename _ElementType>
void _Node<_ElementType>::InsertLink(unsigned int i, unsigned int l) {
  _links.insert(_links.begin() + i, l);
}

template <typename _ElementType>
void _Node<_ElementType>::InsertChildrenCnt(unsigned int i, unsigned int c) {
  _children_cnts.insert(_children_cnts.begin() + i, c);
}

template <typename _ElementType>
void _Node<_ElementType>::InsertTrio(unsigned int i,
                                     const _ElementType &e,
                                     unsigned int l,
                                     unsigned int cc) {
  Insert(i, e);
  InsertLink(i, l);
  InsertChildrenCnt(i, cc);
}

////////////////////////////////////////////////////////////////////////////////
// Extracts                                                                   //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_ElementType _Node<_ElementType>::Extract(unsigned int i) {
  _ElementType element = _elements[i];
  _elements.erase(_elements.begin() + i);
  --_info._elements_cnt;
  return element;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractLinkAfter(unsigned int i) {
  unsigned int index = _links[i + 1];
  _links.erase(_links.begin() + i + 1);
  return index;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractLinkBefore(unsigned int i) {
  unsigned int index = _links[i];
  _links.erase(_links.begin() + i);
  return index;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractChildrenCntAfter(unsigned int i) {
  unsigned int index = _children_cnts[i + 1];
  _children_cnts.erase(_children_cnts.begin() + i + 1);
  return index;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractChildrenCntBefore(unsigned int i) {
  unsigned int index = _children_cnts[i];
  _children_cnts.erase(_children_cnts.begin() + i);
  return index;
}

////////////////////////////////////////////////////////////////////////////////
// Extract back                                                               //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractBackChildrenCnt() {
  unsigned int cnt = _children_cnts.back();
  _children_cnts.pop_back();
  return cnt;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractBackLink() {
  unsigned int index = _links.back();
  _links.pop_back();
  return index;
}

template <typename _ElementType>
_ElementType _Node<_ElementType>::ExtractBack() {
  _ElementType element = _elements.back();
  _elements.pop_back();
  --_info._elements_cnt;
  return element;
}

////////////////////////////////////////////////////////////////////////////////
// Separators                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_Node<_ElementType> _Node<_ElementType>::NodeFromFirstHalf() {
  uint32_t first_half_flags = this->_info._flags;
  first_half_flags = first_half_flags & ~(4UL);  // Set rightest_flag to false
  return _Node<_ElementType> (
    std::vector<_ElementType>(_elements.begin(),
                              _elements.begin() + _elements.size() / 2),
    std::vector<unsigned int>(_links.begin(),
                              _links.begin() + _links.size() / 2),
    std::vector<unsigned int>(_children_cnts.begin(),
                              _children_cnts.begin() + _children_cnts.size() / 2),
    first_half_flags
  );
}

template <typename _ElementType>
_Node<_ElementType> _Node<_ElementType>::NodeFromSecondHalf() {
  return _Node<_ElementType>(
    std::vector<_ElementType>(_elements.end() - _elements.size() / 2,
                              _elements.end()),
    std::vector<unsigned int>(_links.end() - _links.size() / 2,
                              _links.end()),
    std::vector<unsigned int>(_children_cnts.end() - _children_cnts.size() / 2,
                              _children_cnts.end()),
    this->_info._flags
  );
}

template <typename _ElementType>
_ElementType _Node<_ElementType>::GetMiddleElement() {
  return *(_elements.begin() + _elements.size() / 2);
}

template <typename _ElementType>
void _Node<_ElementType>::ConnectWith(_ElementType e,
                                      const _Node<_ElementType> &other) {
  _elements.push_back(e);
  _elements.insert(_elements.end(),
                   other._elements.begin(), other._elements.end());
  _links.insert(_links.end(), other._links.begin(), other._links.end());
  _children_cnts.insert(_children_cnts.begin(),
                        other._children_cnts.begin(),
                        other._children_cnts.end());
}

////////////////////////////////////////////////////////////////////////////////
// Flags setters and getters                                                  //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
bool _Node<_ElementType>::GetIsRoot() const {
  return _info._flags & 1;
}

template <typename _ElementType>
void _Node<_ElementType>::SetIsRoot(bool flag_to_set) {
  _info._flags ^= (-flag_to_set ^ _info._flags) & 1UL;
}

template <typename _ElementType>
bool _Node<_ElementType>::GetIsLeaf() const {
  return _info._flags & 2;  // 2 is 10_2
}

template <typename _ElementType>
void _Node<_ElementType>::SetIsLeaf(bool flag_to_set) {
  _info._flags ^= (-flag_to_set ^ _info._flags) & (1UL << 1);
}

template <typename _ElementType>
bool _Node<_ElementType>::GetIsRightestLeaf() const {
  return _info._flags & 4;
}

template <typename _ElementType>
void _Node<_ElementType>::SetIsRightestLeaf(bool flag_to_set) {
  _info._flags ^= (-flag_to_set ^ _info._flags) & (1UL << 2);
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetAllChildrenCnt() {
  unsigned int sum = 0;
  for (auto i: _children_cnts) {
    sum += i;
  }
  sum += _elements.size();
  return sum;
}

////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
size_t _Node<_ElementType>::Size() const {
  return _elements.size();
}

////////////////////////////////////////////////////////////////////////////////
// Friend functions                                                           //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
_Node<ElementType> Connect(const _Node<ElementType> &left_node,
                           const _Node<ElementType> &right_node,
                           const ElementType e) {
  _Node<ElementType> node_to_return = left_node;
  node_to_return.ConnectWith(e, right_node);
  return node_to_return;
};


#endif //B_TREE_LIST_LIB__NODE_HPP_
