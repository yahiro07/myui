#pragma once

#include <cstdint>

namespace myui {

class DrawingContext {
public:
  virtual ~DrawingContext() = default;
  virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void strokeRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void fillCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void strokeCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void save() = 0;
  virtual void restore() = 0;
  virtual void translate(float x, float y) = 0;
  virtual void rotate(float degree) = 0;
  virtual void scale(float sx, float sy) = 0;

  // expose blend2d context directly in development phase, not for production
  // use
  virtual void *devGetBlend2dContext() = 0; // returning BLContext*
};

struct ImageData {
  const uint8_t *buffer = nullptr;
  int width = 0;
  int height = 0;
  int strideBytes = 0;
};

} // namespace myui