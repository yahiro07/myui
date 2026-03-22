#pragma once
#include "internal-types.h"
#include <source_location>

namespace briq::internal {

inline size_t createBoxId(const std::source_location &loc, int index) {
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

static void createLocalInputState(InputState &input, InputState &gInputState,
                                  internal::NodeBox &box, bool centered) {
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

} // namespace briq::internal