#pragma once
#include <cstdint>

namespace myui {

struct ImageData {
  uint32_t *buffer;
  int width;
  int height;
};

enum class PointerEventType {
  Down,
  Move,
  Up,
};

struct PointerEvent {
  PointerEventType type;
  int x;
  int y;
  int buttons; // 1:left, 2:center, 4:right
};

} // namespace myui