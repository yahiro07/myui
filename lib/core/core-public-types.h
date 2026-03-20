#pragma once
#include <blend2d/blend2d.h>
#include <cstdint>
#include <functional>

namespace myui {

class DrawingContext {
public:
  virtual ~DrawingContext() = default;
  virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void strokeRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void fillCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void strokeCircle(int cx, int cy, int r, uint32_t color) = 0;

  // expose blend2d context directly in development phase, not for production
  // use
  virtual BLContext &devGetBlend2dContext() = 0;
};

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

enum UiLayoutMode { LA_Default, LA_HStack, LA_VStack, LA_HCentered };

} // namespace myui

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