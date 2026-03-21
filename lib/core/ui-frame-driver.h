#pragma once
#include "ui-actor.h"

namespace myui {

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

  void handlePointerEventInput(const PointerEvent &e) {
    internal::affectPointerEventToInputState(bus.gInputState, e);
  }

  void updatePointerStateOnFrameEnd() {
    internal::updateInputStateOnFrameEnd(bus.gInputState);
  }
};

} // namespace myui