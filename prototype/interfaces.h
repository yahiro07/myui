#pragma once
#include <blend2d/blend2d.h>
#include <cstdint>
#include <functional>

namespace myui {

class IDrawingContext {
public:
  virtual ~IDrawingContext() = default;
  virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void strokeRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void fillCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void strokeCircle(int cx, int cy, int r, uint32_t color) = 0;

  virtual BLContext &devGetBlend2dContext() = 0;
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
  int buttons;
};

class IWindowFloor {
public:
  virtual ~IWindowFloor() = default;

  virtual IDrawingContext &getDrawingContext() = 0;

  virtual void setRenderCallback(std::function<void()> callback) = 0;
  virtual void clearRenderCallback() = 0;

  virtual void
  subscribePointer(std::function<void(const PointerEvent &)> callback) = 0;
  virtual void unsubscribePointer() = 0;

  virtual void run() = 0;
};

IWindowFloor *createWindowFloor();

} // namespace myui