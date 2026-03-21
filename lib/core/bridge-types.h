#pragma once

namespace myui::internal {
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
