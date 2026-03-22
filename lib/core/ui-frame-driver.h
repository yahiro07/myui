#pragma once
#include "bridge-types.h"
#include "ui-actor.h"

namespace myui {

static void affectPointerEventToInputState(InputState &input,
                                           const internal::PointerEvent &e) {
  input.x = e.x;
  input.y = e.y;
  input.buttons = e.buttons;
  input.pressed = (e.type == internal::PointerEventType::Down);
  input.released = (e.type == internal::PointerEventType::Up);
  input.hold = (e.buttons != 0);
}

static void updateInputStateOnFrameEnd(InputState &input) {
  input.pressed = false;
  input.released = false;
  input.prevX = input.x;
  input.prevY = input.y;
}

class UiFrameDriver {
private:
  using UiTreeConstructionBus = internal::TreeBuilderBus;
  using LayoutPassOps = internal::LayoutPassOps;
  using PaintPassOps = internal::PaintPassOps;
  using UiProgramFn = internal ::UiProgramFn;

private:
  DrawingContext &dc;
  UiTreeConstructionBus bus{dc};
  LayoutPassOps layoutPassOps{bus};
  PaintPassOps paintPassOps{bus};
  std::unordered_map<size_t, internal::NodeBox> cachedBoxes;

public:
  UiFrameDriver(DrawingContext &dc) : dc(dc) {}
  UiFrameDriver(const UiFrameDriver &) = delete;
  UiFrameDriver &operator=(const UiFrameDriver &) = delete;

private:
public:
  void runFrame(UiProgramFn uiProgramFn, int w, int h) {
    if (bus.debugFirstFrame) {
      printf("begin initial frame\n");
    }
    UiActor ui1{layoutPassOps};
    layoutPassOps.run(uiProgramFn, ui1, w, h, cachedBoxes);
    UiActor ui2{paintPassOps};
    paintPassOps.run(uiProgramFn, ui2, w, h, cachedBoxes);
    updatePointerStateOnFrameEnd();
    if (bus.debugFirstFrame) {
      printf("end initial frame\n");
    }
    bus.debugFirstFrame = false;
  }

  void handlePointerEventInput(const internal::PointerEvent &e) {
    affectPointerEventToInputState(bus.gInputState, e);
  }

  void updatePointerStateOnFrameEnd() {
    updateInputStateOnFrameEnd(bus.gInputState);
  }
};

} // namespace myui