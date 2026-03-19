#include "interfaces.h"
#include <blend2d/blend2d.h>
#include <cstdio>
#include <deque>
#include <functional>
#include <source_location>
#include <stack>

namespace myui {

struct InputState {
  int x;
  int y;
  int buttons; // 1:left, 2:center, 4:right
  bool pressed;
  bool hold;
  bool released;
  int prevX;
  int prevY;
  int deltaX;
  int deltaY;
};

inline size_t makeId(const std::source_location &loc, int index) {
  size_t h = 1469598103934665603ull;
  size_t m = 1099511628211ull;

  h ^= reinterpret_cast<size_t>(loc.file_name());
  h *= m;

  h ^= loc.line();
  h *= m;

  h ^= loc.column();
  h *= m;

  h ^= index;
  h *= m;

  return h;
}

enum class NodeLayoutMode { Default, HStack, VStack, HCentered };

struct Node {
  uint64_t id;
  int w;
  int h;
  NodeLayoutMode layout;
  int gap = 0;
  Node *firstChild = nullptr;
  Node *lastChild = nullptr;
  Node *nextSibling = nullptr;
  std::function<void(IDrawingContext &dc, InputState &input)> drawFn = nullptr;
  bool drawCentered = false;
};

struct NodeBox {
  uint64_t id;
  int x;
  int y;
  int w;
  int h;
};

inline NodeBox createNodeBox(uint64_t id, int x, int y, int w, int h) {
  return {id, x, y, w, h};
}

static void makeLocalInputState(InputState &input, InputState &gInputState,
                                NodeBox &box, bool centered) {
  auto hit = (gInputState.x >= box.x && gInputState.x < box.x + box.w &&
              gInputState.y >= box.y && gInputState.y < box.y + box.h);
  input.x = gInputState.x - box.x;
  input.y = gInputState.y - box.y;
  input.buttons = 0;
  input.pressed = false;
  input.released = false;
  input.hold = false;
  if (hit) {
    input.buttons = gInputState.buttons;
    input.pressed = gInputState.pressed;
    input.released = gInputState.released;
    input.hold = gInputState.hold;
  }
  input.prevX = gInputState.prevX - box.x;
  input.prevY = gInputState.prevY - box.y;
  input.deltaX = input.x - input.prevX;
  input.deltaY = input.y - input.prevY;

  if (centered) {
    input.x -= box.w / 2;
    input.y -= box.h / 2;
    input.prevX -= box.w / 2;
    input.prevY -= box.h / 2;
  }
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

  if (parentNode->layout == NodeLayoutMode::HCentered) {
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
      auto box = createNodeBox(node->id, posX, yCenter - h / 2, w, h);
      destFn(node, box);
      posX += w;
      posX += parentNode->gap;
    });

  } else {
    auto posX = parentBox.x;
    auto posY = parentBox.y;
    auto node = parentNode->firstChild;
    while (node) {
      auto w = node->w == -1 ? parentBox.w : node->w;
      auto h = node->h == -1 ? parentBox.h : node->h;
      auto box = createNodeBox(node->id, posX, posY, w, h);
      destFn(node, box);
      posX += w;
      node = node->nextSibling;
    }
  }
}

static void flushLayout(Node *rootNode,
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

  NodeBox rootBox = createNodeBox(rootNode->id, 0, 0, rootNode->w, rootNode->h);
  applyLayout(rootNode, rootBox);
}

class TreeBuilder {
public:
  Node *rootNode = nullptr;
  Node *currentParent = nullptr;
  std::stack<Node *> parentStack;

  void reset() {
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

class NodeHandle;

class UiActor {
private:
  IDrawingContext &dc;

  InputState gInputState{};

  int seqNoteIdCounter = 0;
  TreeBuilder treeBuilder;
  std::deque<Node> nodeList;

public:
  bool debugFirstFrame = true;

  UiActor(IDrawingContext &dc) : dc(dc) {}
  UiActor(const UiActor &) = delete;
  UiActor &operator=(const UiActor &) = delete;

  void beginFrame() {
    if (debugFirstFrame) {
      printf("beginFrame\n");
    }
    seqNoteIdCounter = 0;
    treeBuilder.reset();
  }

  void endFrame() {
    if (treeBuilder.rootNode) {
      flushLayout(treeBuilder.rootNode, [&](Node *node, NodeBox &box) {
        if (node->drawFn) {
          drawNode(node, box);
        }
      });
    }
    if (debugFirstFrame) {
      printf("endFrame\n");
    }
    nodeList.clear();
  }

  NodeHandle
  rootBox(int w, int h, NodeLayoutMode layout = NodeLayoutMode::Default,
          std::source_location loc = std::source_location::current());

  NodeHandle box(int w, int h, NodeLayoutMode layout = NodeLayoutMode::Default,
                 std::source_location loc = std::source_location::current());

  template <class F> void sub(F &&fn, Node *newParent) {
    treeBuilder.pushParent(newParent);
    fn();
    treeBuilder.popParent();
  }

private:
  Node *createNode(uint64_t id, int w, int h, NodeLayoutMode layout) {
    nodeList.push_back({id, w, h, layout});
    return &nodeList.back();
  }

  void drawNode(Node *node, NodeBox &box) {
    auto centered = node->drawCentered;
    dc.strokeRect(box.x, box.y, box.w, box.h, 0x88888888);

    auto bl = dc.devGetBlend2dContext();
    InputState input;
    makeLocalInputState(input, gInputState, box, centered);
    bl.save();
    if (centered) {
      bl.translate(box.x + (float)box.w / 2, box.y + (float)box.h / 2);
    } else {
      bl.translate(box.x, box.y);
    }
    node->drawFn(dc, input);
    bl.restore();
  }

public:
  void handlePointerEventInput(const PointerEvent &e) {
    gInputState.x = e.x;
    gInputState.y = e.y;
    gInputState.buttons = e.buttons;
    gInputState.pressed = (e.type == PointerEventType::Down);
    gInputState.released = (e.type == PointerEventType::Up);
    gInputState.hold = (e.buttons != 0);
  }

  void updatePointerStateOnFrameEnd() {
    gInputState.pressed = false;
    gInputState.released = false;
    gInputState.prevX = gInputState.x;
    gInputState.prevY = gInputState.y;
  }
};

template <class> inline constexpr bool always_false_v = false;

class NodeHandle {
private:
  UiActor &owner;
  Node &node;

public:
  NodeHandle(UiActor &owner, Node &node) : owner(owner), node(node) {}

  NodeHandle &hCenter(int gap = 0) {
    node.layout = NodeLayoutMode::HCentered;
    node.gap = gap;
    return *this;
  }

  template <class F> NodeHandle &sub(F &&fn) {
    owner.sub(fn, &node);
    return *this;
  }

private:
  template <class F>
  std::function<void(IDrawingContext &, InputState &)> wrapDrawFn(F &&fn) {
    if constexpr (requires {
                    fn(std::declval<IDrawingContext &>(),
                       std::declval<InputState &>());
                  }) {
      return std::forward<F>(fn);
    } else if constexpr (requires { fn(std::declval<IDrawingContext &>()); }) {
      return [stored = std::forward<F>(fn)](IDrawingContext &dc,
                                            InputState &input) mutable {
        stored(dc);
      };
    } else {
      // static_assert(false, "draw() requires fn(dc, input) or fn(dc)");
      static_assert(always_false_v<F>,
                    "draw() requires fn(dc, input) or fn(dc)");
    }
  }

public:
  template <class F> NodeHandle &draw(F &&fn) {
    node.drawFn = wrapDrawFn(std::forward<F>(fn));
    node.drawCentered = false;
    return *this;
  }
  template <class F> NodeHandle &drawC(F &&fn) {
    node.drawFn = wrapDrawFn(std::forward<F>(fn));
    node.drawCentered = true;
    return *this;
  }
};

NodeHandle UiActor::rootBox(int w, int h, NodeLayoutMode layout,
                            std::source_location loc) {
  auto boxId = makeId(loc, seqNoteIdCounter++);
  auto node = createNode(boxId, w, h, layout);
  treeBuilder.setRootNode(node);
  return NodeHandle(*this, *node);
}

NodeHandle UiActor::box(int w, int h, NodeLayoutMode layout,
                        std::source_location loc) {
  auto boxId = makeId(loc, seqNoteIdCounter++);
  auto node = createNode(boxId, w, h, layout);
  treeBuilder.linkNodeToParentAndSiblings(node);
  return NodeHandle(*this, *node);
}

} // namespace myui