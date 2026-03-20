#include "ui-actor.h"
#include <blend2d/blend2d.h>

namespace myui {

using Node = internal::Node;
using NodeBox = internal::NodeBox;

void drawNodeWithTranslate(DrawingContext &dc, Node *node, NodeBox &box,
                           InputState &gInputState) {
  auto centered = node->drawCentered;
  dc.strokeRect(box.x, box.y, box.w, box.h, 0x88888888);

  InputState input;
  createLocalInputState(input, gInputState, box, centered);

  BLContext *bl = (BLContext *)dc.devGetBlend2dContext();
  bl->save();
  if (centered) {
    bl->translate(box.x + (float)box.w / 2, box.y + (float)box.h / 2);
  } else {
    bl->translate(box.x, box.y);
  }
  node->drawFn(dc, input);
  bl->restore();
}

} // namespace myui