#pragma once
#include "internal-helper.h"
#include <cstdio>

namespace myui::internal {

template <class F>
void drawCoreStatic(F &&fn, internal::Node *node, bool centered,
                    DrawingContext &dc, internal::NodeBox &cachedBox,
                    InputState &gInputState, bool debugFirstFrame) {
  if (cachedBox.skip)
    return;
  auto cb = cachedBox;
  if (debugFirstFrame) {
    printf("draw with cached rect: %llu %d %d %d %d\n", node->id, cb.x, cb.y,
           cb.w, cb.h);
  }
  dc.strokeRect(cb.x, cb.y, cb.w, cb.h, 0x88888888);

  InputState input;
  createLocalInputState(input, gInputState, cb, centered);
  dc.save();
  if (centered) {
    dc.translate(cachedBox.x + (float)cachedBox.w / 2,
                 cachedBox.y + (float)cachedBox.h / 2);
  } else {
    dc.translate(cachedBox.x, cachedBox.y);
  }
  if constexpr (requires { fn(dc, input); }) {
    fn(dc, input);
  } else if constexpr (requires { fn(dc); }) {
    fn(dc);
  }
  dc.restore();
}

} // namespace myui::internal