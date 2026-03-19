#pragma once
#include "common-types.h"
#include "dc.h"
#include <functional>
#include <memory>

namespace myui {

class UiActor {
  DrawingContext *dc;

public:
  void setDrawingContext(DrawingContext *dc) { this->dc = dc; }
  UiActor &box() { return *this; };
  UiActor &draw(std::function<void(DrawingContext &)> callback) {
    return *this;
  }
  void handlePointerEventInput(const PointerEvent &e) {};
};

} // namespace myui
