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

  BTreeList(size_t size = 0, size_t limit = 256);

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
};

////////////////////////////////////////////////////////////////////////////////
// Implementation                                                             //
////////////////////////////////////////////////////////////////////////////////

template <typename ElementType>
BTreeList<ElementType>::BTreeList(size_t size, size_t limit)
  : _size(size),
    _file_manager("./data"),
    _in_memory_node() {
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

  do {
    curr_node = _file_manager.GetNode(curr_file_pos);
    unsigned int child_index = 0;
    while (
        indexes_on_the_left + curr_node._children_cnts[child_index] + 1 < index
    ) {
      indexes_on_the_left += curr_node._children_cnts[child_index] + 1;
      ++child_index;
    }
    if (!curr_node._list_flag) {
      curr_file_pos = curr_node._links[child_index];
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
    curr_node._elements.insert(curr_node._elements.begin() + element_index,
                               element_to_insert);
    curr_node._links[element_index] = link_before_inserted; // Just change link
    curr_node._links.insert(curr_node._links.begin() + element_index + 1,
                            link_after_inserted);

    if (curr_node._type != NODE_TYPES::ROOT) {
      if (curr_node._elements.size() >=
          _file_manager._node_params._elements_cnt) {     // insertion into regular node
        element_to_insert = curr_node._elements[/*middle_element*/];
        _Node<ElementType> new_node_to_add(
            std::vector<ElementType>(
                curr_node._elements.begin() + curr_node._elements.size() / 2
                    + 1, // From next after middle element to the end
                curr_node._elements.end()
            ),
            std::vector<unsigned int>(
                curr_node._links.begin() + curr_node._links.size() / 2
                    + 1, // From link after middle element to the end
                curr_node._links.end()
            ),
            std::vector<unsigned int>(
                curr_node._children_cnts.begin()
                    + curr_node._children_cnts.size() / 2
                    + 1, // From link after middle elemet to the end
                curr_node._children_cnts.end()
            )
        );  // TODO: be sure about edges
        curr_node = _Node<ElementType>(
            std::vector<ElementType>(
                curr_node._elements.begin(),  // From beginning to the middle without include
                curr_node._elements.begin() + curr_node._elements.size() / 2 + 1
            ),
            std::vector<unsigned int>(
                curr_node._links.begin(),  // From beginning to the link before middle
                curr_node._links.end() + curr_node._links.size() / 2 + 1
            ),
            std::vector<unsigned int>(  // From beginning to the link before middle
                curr_node._children_cnts.begin(),
                curr_node._children_cnts.end()
                    + curr_node._children_cnts.size() / 2 + 1
            )
        );
        link_after_inserted = _file_manager._AddNewNode(new_node_to_add);
        link_before_inserted = *(path_indexes.rbegin());
        path_indexes.pop_back();

        // curr node goes upper with element to insert
        // changed to middle element of curr node
        // go upper
      }
    } else {  // insertion into ROOT

    }
  }
}

template<typename ElementType>
size_t BTreeList<ElementType>::Size() const {
  return _size;
}

#endif //B_TREE_LIST_LIBRARY_H