#pragma once
#include "pass-ops.h"

namespace myui {

class NodeHandle {
private:
  using Node = internal::Node;
  using PassOps = internal::PassOps;
  using PaintPassOps = internal::PaintPassOps;

private:
  PassOps &passOps;
  Node &node;

public:
  NodeHandle(Node &node, PassOps &passOps) : node(node), passOps(passOps) {}

private:
  void setNodeLayout(Node &node, UiLayoutMode layout, int gap) {
    node.layout = layout;
    node.gap = gap;
  }

public:
  NodeHandle &hCenter(int gap = 0) {
    setNodeLayout(node, LA_HCentered, gap);
    return *this;
  }

public:
  template <class F> NodeHandle &sub(F &&fn) {
    passOps.pushParent(&node);
    fn();
    passOps.popParent();
    return *this;
  }

private:
  template <class F> void drawCore(F &&fn, Node *node, bool centered) {
    if (PaintPassOps *paintOps = dynamic_cast<PaintPassOps *>(&passOps)) {
      // draw only when paint phase
      paintOps->drawCore(fn, node, centered);
    }
  }

public:
  template <class F> NodeHandle &draw(F &&fn) {
    drawCore(fn, &node, false);
    return *this;
  }
  template <class F> NodeHandle &drawC(F &&fn) {
    drawCore(fn, &node, true);
    return *this;
  }
};

} // namespace myui