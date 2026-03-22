#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class BLContext;

namespace briq {

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
  virtual void drawText(std::string text, int ox, int oy, std::string fontKey,
                        int size, uint32_t color, bool centered) = 0;
  // expose blend2d context directly in development phase, not for production
  // use
  virtual BLContext *devGetBlend2dContext() = 0; // returning BLContext*
};

} // namespace briq