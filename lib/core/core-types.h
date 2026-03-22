#pragma once
#include "../drawings/drawing-types.h"

namespace briq {

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

} // namespace briq
