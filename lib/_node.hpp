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
        const std::vector<unsigned int>& children_cnts);

  void Set(const _ElementType& e, unsigned int i);

  void SetLink(unsigned int l, unsigned int i);

  void SetChildrenCnt(unsigned int cc, unsigned int i);

  void PushBack(const _ElementType& e);

  void PushBackLink(unsigned int l);

  void PushBackChildrenCnt(unsigned int cc);

  _ElementType Get(unsigned int i) const;

  unsigned int GetLink(unsigned int i) const;

  unsigned int GetChildrenCnt(unsigned int i) const;

  void Insert(const _ElementType& e, unsigned int i);

  void InsertLink(unsigned int l, unsigned int i);

  void InsertChildrenCnt(unsigned int c, unsigned int i);

  unsigned int ExtractChildrenCnt(unsigned int i);

  unsigned int ExtractLink(unsigned int i);

  _ElementType Extract(unsigned int i);

  unsigned int ExtractBackChildrenCnt();

  unsigned int ExtractBackLink();

  _ElementType ExtractBack();

  _Node<_ElementType> NodeFromFirstHalf();

  _Node<_ElementType> NodeFromSecondHalf();

  _ElementType GetMiddleElement();

  void ConnectWith(_ElementType e, const _Node<_ElementType> &other);

  bool IsRoot() const;

  void SetIsRoot(bool flag_to_set);

  bool IsLeaf() const;

  void SetIsLeaf(bool flag_to_set);

  //////////////////////////////////////////////////////////////////////////////
  // Parameters structs declaration                                           //
  //////////////////////////////////////////////////////////////////////////////

  struct _NodeInfo{
    int _parent_link;
    size_t _elements_cnt;
  };

  //////////////////////////////////////////////////////////////////////////////
  // Fields                                                                   //
  //////////////////////////////////////////////////////////////////////////////

  uint32_t _flags;
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
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_Node<_ElementType>::_Node() : _elements(std::vector<_ElementType>(0)) {}

template <typename _ElementType>
_Node<_ElementType>::_Node(
    const std::vector<_ElementType>& v,
    const std::vector<unsigned int>& links,
    const std::vector<unsigned int>& children_cnts)
  : _elements(v),
    _links(links),
    _children_cnts(children_cnts) {}

////////////////////////////////////////////////////////////////////////////////
// Setters                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::Set(const _ElementType &e, unsigned int i) {
  _elements[i] = e;
}

template <typename _ElementType>
void _Node<_ElementType>::SetLink(unsigned int l, unsigned int i) {
  _links[i] = l;
}

template <typename _ElementType>
void _Node<_ElementType>::SetChildrenCnt(unsigned int cc, unsigned int i) {
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
unsigned int _Node<_ElementType>::GetLink(unsigned int i) const {
  return _links[i];
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::GetChildrenCnt(unsigned int i) const {
  return _children_cnts[i];
}

////////////////////////////////////////////////////////////////////////////////
// Push backs                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::PushBack(const _ElementType &e) {
  _elements.push_back(e);
}

template <typename _ElementType>
void _Node<_ElementType>::PushBackLink(unsigned int l) {
  _links.push_back(l);
}

template <typename _ElementType>
void _Node<_ElementType>::PushBackChildrenCnt(unsigned int cc) {
  _children_cnts.push_back(cc);
}

////////////////////////////////////////////////////////////////////////////////
// Inserts                                                                    //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
void _Node<_ElementType>::Insert(const _ElementType &e, unsigned int i) {
  _elements.insert(_elements.begin() + i, e);
  _info._elements_cnt = _elements.size();
}

template <typename _ElementType>
void _Node<_ElementType>::InsertLink(unsigned int l, unsigned int i) {
  _links.insert(_links.begin() + i, l);
  _info._elements_cnt = _elements.size();
}

template <typename _ElementType>
void _Node<_ElementType>::InsertChildrenCnt(unsigned int c, unsigned int i) {
  _children_cnts.insert(_children_cnts.begin() + i, c);
  _info._elements_cnt = _elements.size();
}

////////////////////////////////////////////////////////////////////////////////
// Extracts                                                                   //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractChildrenCnt(unsigned int i) {
  unsigned int cnt = _children_cnts[i];
  _children_cnts.erase(_children_cnts.begin() + i);
  _info._elements_cnt = _elements.size();
  return cnt;
}

template <typename _ElementType>
unsigned int _Node<_ElementType>::ExtractLink(unsigned int i) {
  unsigned int index = _links[i];
  _links.erase(_links.begin() + i);
  _info._elements_cnt = _elements.size();
  return index;
}

template <typename _ElementType>
_ElementType _Node<_ElementType>::Extract(unsigned int i) {
  _ElementType element = _elements[i];
  _elements.erase(_elements.begin() + i);
  _info._elements_cnt = _elements.size();
  return element;
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
  return element;
}

////////////////////////////////////////////////////////////////////////////////
// Separators                                                                 //
////////////////////////////////////////////////////////////////////////////////

template <typename _ElementType>
_Node<_ElementType> _Node<_ElementType>::NodeFromFirstHalf() {
  return _Node<_ElementType> {
    _elements.begin(), _elements.begin() + _elements.size() / 2,
    _links.begin(), _links.begin() + _links.size() / 2,
    _children_cnts.begin(), _children_cnts.size() / 2
  };
}

template <typename _ElementType>
_Node<_ElementType> _Node<_ElementType>::NodeFromSecondHalf() {
  return _Node<_ElementType> {
    _elements.end() - _elements.size() / 2, _elements.end(),
    _links.end() - _links.size() / 2, _links.end(),
    _children_cnts.end() - _children_cnts.size() / 2, _children_cnts.size()
  };
}

template <typename _ElementType>
_ElementType _Node<_ElementType>::GetMiddleElement() {
  return _elements.begin() + _elements.size() / 2 + 1;
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

template <typename _ElementType>
bool _Node<_ElementType>::IsRoot() const {
  return _flags & 1;
}

template <typename _ElementType>
void _Node<_ElementType>::SetIsRoot(bool flag_to_set) {
  _flags ^= 1;
}

template <typename _ElementType>
bool _Node<_ElementType>::IsLeaf() const {
  return _flags & 2;  // 2 is 10_2
}

template <typename _ElementType>
void _Node<_ElementType>::SetIsLeaf(bool flag_to_set) {
  _flags ^= 2;  // 2 is 10_2
}

#endif //B_TREE_LIST_LIB__NODE_HPP_
