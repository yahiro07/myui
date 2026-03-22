#pragma once
#include "core-types.h"

namespace briq::internal {
struct Node {
  uint64_t id;
  int w;
  int h;
  UiLayoutMode layout;
  int gap = 0;
  Node *firstChild = nullptr;
  Node *lastChild = nullptr;
  Node *nextSibling = nullptr;
  bool skip = false;
};

struct NodeBox {
  uint64_t id;
  int x;
  int y;
  int w;
  int h;
  bool skip;
};

} // namespace briq::internal
