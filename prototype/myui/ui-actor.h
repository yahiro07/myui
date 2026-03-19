#pragma once
#include "node-handle.h"
#include "public-types.h"
#include "window-floor.h"
#include <blend2d/blend2d.h>
#include <deque>
#include <memory>
#include <source_location>

namespace myui {

struct NodeBox;

class UiActor {
public:
  bool debugFirstFrame = true;
  UiActor(DrawingContext &dc);
  ~UiActor();
  UiActor(const UiActor &) = delete;
  UiActor &operator=(const UiActor &) = delete;

private:
  DrawingContext &dc;
  InputState gInputState{};
  int seqNoteIdCounter = 0;
  std::unique_ptr<TreeBuilder> treeBuilder;
  std::unique_ptr<std::deque<Node>> nodeList;

private:
  Node *createNode(uint64_t id, int w, int h, UiLayoutMode layout);
  void drawNode(Node *node, NodeBox &box);

public:
  void beginFrame();
  void endFrame();

  NodeHandle
  rootBox(int w, int h, UiLayoutMode layout = LA_Default,
          std::source_location loc = std::source_location::current());

  NodeHandle box(int w, int h, UiLayoutMode layout = LA_Default,
                 std::source_location loc = std::source_location::current());

  void handlePointerEventInput(const PointerEvent &e);
  void updatePointerStateOnFrameEnd();
};

} // namespace myui