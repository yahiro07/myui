#include "common-types.h"
#include <memory>

namespace myui {

class DrawingContext {
public:
  virtual ~DrawingContext() = default;
  virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void strokeRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void fillCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void strokeCircle(int cx, int cy, int r, uint32_t color) = 0;

  virtual ImageData &getImageData() const = 0;
};
std::unique_ptr<DrawingContext> createDrawingContext(); // blend2d

} // namespace myui