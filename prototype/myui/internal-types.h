#pragma once
#include "public-types.h"
#include <functional>

namespace myui {

struct Node {
  uint64_t id;
  int w;
  int h;
  UiLayoutMode layout;
  int gap = 0;
  Node *firstChild = nullptr;
  Node *lastChild = nullptr;
  Node *nextSibling = nullptr;
  std::function<void(DrawingContext &dc, InputState &input)> drawFn = nullptr;
  bool drawCentered = false;
};

struct NodeBox {
  uint64_t id;
  int x;
  int y;
  int w;
  int h;
};

} // namespace myui