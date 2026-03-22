#pragma once
#include "draw-core-static.h"
#include "layouter.h"
#include "tree-builder-bus.h"

namespace briq {
class UiActor;
}

namespace briq::internal {

using UiProgramFn = std::function<void(UiActor &, int, int)>;
using CachedBoxMap = std::unordered_map<size_t, NodeBox>;

class PassOps {
protected:
  TreeBuilderBus &bus;

public:
  PassOps(TreeBuilderBus &bus) : bus(bus) {}
  virtual ~PassOps() = default;

public:
  void pushParent(Node *node) { bus.pushParent(node); }
  void popParent() { bus.popParent(); }

  Node *createNodeWithId(int w, int h, UiLayoutMode layout,
                         std::source_location loc, bool isRoot) {
    return bus.createNodeWithId(w, h, layout, loc, isRoot);
  }
};

class LayoutPassOps : public PassOps {
public:
  LayoutPassOps(TreeBuilderBus &bus) : PassOps(bus) {}

  // for layout pass, layout is computed and output to cachedBoxes
  void run(UiProgramFn uiProgramFn, UiActor &uiActor, int w, int h,
           CachedBoxMap &cachedBoxes) { // output
    cachedBoxes.clear();
    bus.clearNodes();

    uiProgramFn(uiActor, w, h); // 1st evaluation pass for layout

    auto rootNode = bus.getRootNode();
    if (rootNode) {
      flushLayout(rootNode,
                  [&](Node *node, NodeBox &box) { cachedBoxes[box.id] = box; });
    }
  }
};

class PaintPassOps : public PassOps {
public:
  CachedBoxMap *cachedBoxes;

  PaintPassOps(TreeBuilderBus &bus) : PassOps(bus) {}

  // for paint pass, draw UI using cachedBoxes from layout pass
  void run(UiProgramFn uiProgramFn, UiActor &uiActor, int w, int h,
           CachedBoxMap &cachedBoxes) { // input
    this->cachedBoxes = &cachedBoxes;
    bus.clearNodes();

    uiProgramFn(uiActor, w, h); // 2nd evaluation pass for drawing
  }

  template <class F> void drawCore(F &&fn, Node *node, bool centered) {
    if (!cachedBoxes->contains(node->id))
      return;
    auto cachedBox = cachedBoxes->at(node->id);
    drawCoreStatic(fn, node, centered, bus.dc, cachedBox, bus.gInputState,
                   bus.debugFirstFrame);
  }
};

} // namespace briq::internal