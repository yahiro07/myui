#pragma once
#include <cstdint>

namespace myui::internal {

struct ImageData {
  const uint8_t *buffer = nullptr;
  int width = 0;
  int height = 0;
  int strideBytes = 0;
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

} // namespace myui::internal
