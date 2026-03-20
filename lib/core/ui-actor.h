#pragma once
#include "core-types.h"
#include "internal-helper.h"
#include "layouter.h"
#include "node-handle.h"
#include <deque>
#include <memory>
#include <source_location>

namespace myui {

void drawNodeWithTranslate(DrawingContext &dc, internal::Node *node,
                           internal::NodeBox &box, InputState &gInputState);

class UiActor {
private:
  using Node = internal::Node;
  using NodeBox = internal::NodeBox;
  using TreeBuilder = internal::TreeBuilder;

public:
  bool debugFirstFrame = true;
  UiActor(DrawingContext &dc) : dc(dc) {
    treeBuilder = std::make_unique<TreeBuilder>();
    nodeList = std::make_unique<std::deque<Node>>();
  }
  ~UiActor() = default;
  UiActor(const UiActor &) = delete;
  UiActor &operator=(const UiActor &) = delete;

private:
  DrawingContext &dc;
  InputState gInputState{};
  int seqNoteIdCounter = 0;
  std::unique_ptr<TreeBuilder> treeBuilder;
  std::unique_ptr<std::deque<Node>> nodeList;

private:
  Node *createNode(uint64_t id, int w, int h, UiLayoutMode layout) {
    Node node{id, w, h, layout};
    nodeList->push_back(node);
    return &nodeList->back();
  }

  void drawNode(Node *node, NodeBox &box) {
    drawNodeWithTranslate(dc, node, box, gInputState);
  }

public:
  void beginFrame() {
    if (debugFirstFrame) {
      printf("beginFrame\n");
    }
    seqNoteIdCounter = 0;
    treeBuilder->reset();
  }

  void endFrame() {
    if (treeBuilder->rootNode) {
      flushLayout(treeBuilder->rootNode, [&](Node *node, NodeBox &box) {
        if (node->drawFn) {
          drawNode(node, box);
        }
      });
    }
    if (debugFirstFrame) {
      printf("endFrame\n");
    }
    nodeList->clear();
  }

  NodeHandle
  rootBox(int w, int h, UiLayoutMode layout = LA_Default,
          std::source_location loc = std::source_location::current()) {
    auto boxId = internal::createBoxId(loc, seqNoteIdCounter++);
    auto node = createNode(boxId, w, h, layout);
    treeBuilder->setRootNode(node);
    return NodeHandle(*node, treeBuilder.get());
  }

  NodeHandle box(int w, int h, UiLayoutMode layout = LA_Default,
                 std::source_location loc = std::source_location::current()) {
    auto boxId = internal::createBoxId(loc, seqNoteIdCounter++);
    auto node = createNode(boxId, w, h, layout);
    treeBuilder->linkNodeToParentAndSiblings(node);
    return NodeHandle(*node, treeBuilder.get());
  }

  void handlePointerEventInput(const PointerEvent &e) {
    internal::affectPointerEventToInputState(gInputState, e);
  }

  void updatePointerStateOnFrameEnd() {
    internal::updateInputStateOnFrameEnd(gInputState);
  }
};

} // namespace myui