#pragma once
#include "../core/bridge-types.h"
#include "../core/core-public-types.h"
#include <functional>

namespace myui {

class WindowFloor {
public:
  virtual ~WindowFloor() = default;

  virtual DrawingContext &getDrawingContext() = 0;

  virtual void setRenderCallback(std::function<void()> callback) = 0;
  virtual void clearRenderCallback() = 0;

  virtual void
  subscribePointer(std::function<void(const PointerEvent &)> callback) = 0;
  virtual void unsubscribePointer() = 0;

  virtual void run() = 0;
};

WindowFloor *createWindowFloor();
} // namespace myui