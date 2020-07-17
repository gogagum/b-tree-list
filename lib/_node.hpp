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

  _ElementType Get(unsigned int i) const;

  void Insert(const _ElementType& e, unsigned int i);

  void InsertLink(unsigned int l, unsigned int i);

  void InsertChildrenCnt(unsigned int c, unsigned int i);

  unsigned int ExtractChildrenCnt(unsigned int i);

  unsigned int ExtractLink(unsigned int i);

  _ElementType Extract(unsigned int i);

  bool IsRoot();

  void SetIsRoot(bool flag_to_set);

  bool IsList();

  void SetIsList(bool flag_to_set);

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

  template<typename ElementType>
  friend class BTreeList;
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

template <typename _ElementType>
void _Node<_ElementType>::Set(const _ElementType &e, unsigned int i) {
  _elements[i] = e;
}

template <typename _ElementType>
_ElementType _Node<_ElementType>::Get(unsigned int i) const {
  return _elements[i];
}

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

#endif //B_TREE_LIST_LIB__NODE_HPP_
