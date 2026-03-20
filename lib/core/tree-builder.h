#pragma once
#include "internal-types.h"
#include <stack>

namespace myui::internal {

class TreeBuilder {
public:
  Node *rootNode = nullptr;
  Node *currentParent = nullptr;
  std::stack<Node *> parentStack;

  void clear() {
    rootNode = nullptr;
    currentParent = nullptr;
    parentStack = {};
  }
  void setRootNode(Node *node) {
    rootNode = node;
    currentParent = node;
  }
  void linkNodeToParentAndSiblings(Node *node) {
    if (currentParent != nullptr) {
      if (currentParent->firstChild) {
        currentParent->lastChild->nextSibling = node;
      } else {
        currentParent->firstChild = node;
      }
      currentParent->lastChild = node;
    }
  }
  void pushParent(Node *node) {
    parentStack.push(currentParent);
    currentParent = node;
  }
  void popParent() {
    if (currentParent == nullptr)
      return;
    currentParent = parentStack.top();
    parentStack.pop();
  }
};

} // namespace myui::internal