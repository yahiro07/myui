#include "node-handle.h"
#include "tree-builder.h"

namespace myui {

void NodeHandle::pushParent(Node *node) { treeBuilder->pushParent(node); }

void NodeHandle::popParent() { treeBuilder->popParent(); }

void NodeHandle::setNodeLayout(Node &node, UiLayoutMode layout, int gap) {
  node.layout = layout;
  node.gap = gap;
}
void NodeHandle::setDrawFn(
    Node &node, std::function<void(DrawingContext &, InputState &)> drawFn,
    bool centered) {
  node.drawFn = drawFn;
  node.drawCentered = centered;
}

} // namespace myui