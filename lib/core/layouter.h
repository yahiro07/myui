#pragma once
#include "internal-types.h"
#include <functional>

namespace briq::internal {

inline NodeBox createNodeBox(uint64_t id, int x, int y, int w, int h,
                             bool skip) {
  return {id, x, y, w, h, skip};
}

static void iterChildNodes(Node *parentNode,
                           std::function<void(Node *node)> destFn) {
  auto node = parentNode->firstChild;
  while (node) {
    destFn(node);
    node = node->nextSibling;
  }
}

static void
layoutChildBoxes(Node *parentNode, NodeBox &parentBox,
                 std::function<void(Node *node, NodeBox &)> destFn) {

  if (parentNode->layout == LA_HCentered) {
    auto totalWidth = 0;
    auto childCount = 0;
    iterChildNodes(parentNode, [&](Node *node) {
      totalWidth += node->w;
      childCount++;
    });
    totalWidth += parentNode->gap * (childCount - 1);

    auto posX = parentBox.x + (parentBox.w - totalWidth) / 2;
    auto yCenter = parentBox.y + parentBox.h / 2;
    iterChildNodes(parentNode, [&](Node *node) {
      auto w = node->w == -1 ? parentBox.w : node->w;
      auto h = node->h == -1 ? parentBox.h : node->h;
      auto box =
          createNodeBox(node->id, posX, yCenter - h / 2, w, h, node->skip);
      destFn(node, box);
      posX += w;
      posX += parentNode->gap;
    });

  } else {
    auto posX = parentBox.x;
    auto posY = parentBox.y;
    iterChildNodes(parentNode, [&](Node *node) {
      auto w = node->w == -1 ? parentBox.w : node->w;
      auto h = node->h == -1 ? parentBox.h : node->h;
      auto box = createNodeBox(node->id, posX, posY, w, h, node->skip);
      destFn(node, box);
      posX += w;
    });
  }
}

inline void flushLayout(Node *rootNode,
                        std::function<void(Node *node, NodeBox &)> emit) {
  std::function<void(Node *, NodeBox &)> applyLayout =
      [&](Node *currentNode, NodeBox &currentBox) {
        emit(currentNode, currentBox);
        if (currentNode->firstChild) {
          layoutChildBoxes(
              currentNode, currentBox,
              [&](Node *node, NodeBox &box) { applyLayout(node, box); });
        }
      };

  NodeBox rootBox = createNodeBox(rootNode->id, 0, 0, rootNode->w, rootNode->h,
                                  rootNode->skip);
  applyLayout(rootNode, rootBox);
}

} // namespace briq::internal