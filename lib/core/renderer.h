#include "bridge-types.h"
#include "core-public-types.h"
#include <memory>

namespace myui {

class Blend2dRenderer : public DrawingContext {
public:
  virtual ~Blend2dRenderer() = default;
  virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void strokeRect(int x, int y, int w, int h, uint32_t color) = 0;
  virtual void fillCircle(int cx, int cy, int r, uint32_t color) = 0;
  virtual void strokeCircle(int cx, int cy, int r, uint32_t color) = 0;

  virtual ImageData &getImageData() const = 0;
};

std::unique_ptr<Blend2dRenderer> createBlend2dRenderer();

} // namespace myui