#pragma once
#include "internal-helper.h"
#include "tree-builder.h"
#include <deque>
#include <source_location>
#include <unordered_map>

namespace myui::internal {

class TreeBuilderBus {

private:
  std::unordered_map<size_t, size_t> boxSourceLocationIdToCountMap;
  std::deque<internal::Node> nodeList;
  TreeBuilder treeBuilder;

public:
  bool debugFirstFrame = true;
  DrawingContext &dc;
  InputState gInputState{};

  TreeBuilderBus(DrawingContext &dc) : dc(dc) {}

public:
  Node *getRootNode() { return treeBuilder.getRootNode(); }
  void pushParent(Node *node) { treeBuilder.pushParent(node); }
  void popParent() { treeBuilder.popParent(); }

  void clearNodes() {
    treeBuilder.clear();
    nodeList.clear();
    boxSourceLocationIdToCountMap.clear();
  }

  Node *createNodeWithId(int w, int h, UiLayoutMode layout,
                         std::source_location loc, bool isRoot) {
    auto index = 0;
    if (!isRoot) {
      auto boxLocationId = createBoxId(loc, 0);
      index = boxSourceLocationIdToCountMap[boxLocationId]++;
    }
    auto boxId = createBoxId(loc, index);

    nodeList.emplace_back(boxId, w, h, layout);
    Node *node = &nodeList.back();
    if (isRoot) {
      treeBuilder.setRootNode(node);
    } else {
      treeBuilder.linkNodeToParentAndSiblings(node);
    }
    return node;
  }
};

}; // namespace myui::internal