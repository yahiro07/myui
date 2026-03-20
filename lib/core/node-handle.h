#pragma once
#include "tree-builder.h"
#include <functional>

namespace myui {

template <class> inline constexpr bool always_false_v = false;

struct UiActorRoamingObject {
  internal::TreeBuilder &treeBuilder;
  bool &debugFirstFrame;
};

class NodeHandle {
private:
  using Node = internal::Node;

private:
  Node &node;
  UiActorRoamingObject &ro;

public:
  NodeHandle(Node &node, UiActorRoamingObject &ro) : node(node), ro(ro) {}

private:
  void pushParent(Node *node) { ro.treeBuilder.pushParent(node); }

  void popParent() { ro.treeBuilder.popParent(); }

  void setNodeLayout(Node &node, UiLayoutMode layout, int gap) {
    node.layout = layout;
    node.gap = gap;
  }
  void setDrawFn(Node &node,
                 std::function<void(DrawingContext &, InputState &)> drawFn,
                 bool centered) {
    node.drawFn = drawFn;
    node.drawCentered = centered;
  }

public:
  NodeHandle &hCenter(int gap = 0) {
    setNodeLayout(node, LA_HCentered, gap);
    return *this;
  }

  template <class F> NodeHandle &sub(F &&fn) {
    pushParent(&node);
    fn();
    popParent();
    return *this;
  }

private:
  template <class F>
  std::function<void(DrawingContext &, InputState &)> wrapDrawFn(F &&fn) {
    if constexpr (requires {
                    fn(std::declval<DrawingContext &>(),
                       std::declval<InputState &>());
                  }) {
      return std::forward<F>(fn);
    } else if constexpr (requires { fn(std::declval<DrawingContext &>()); }) {
      return [stored = std::forward<F>(fn)](
                 DrawingContext &dc, InputState &input) mutable { stored(dc); };
    } else {
      // static_assert(false, "draw() requires fn(dc, input) or fn(dc)");
      static_assert(always_false_v<F>,
                    "draw() requires fn(dc, input) or fn(dc)");
    }
  }

public:
  template <class F> NodeHandle &draw(F &&fn) {
    setDrawFn(node, wrapDrawFn(std::forward<F>(fn)), false);
    return *this;
  }
  template <class F> NodeHandle &drawC(F &&fn) {
    setDrawFn(node, wrapDrawFn(std::forward<F>(fn)), true);
    return *this;
  }
};

} // namespace myui