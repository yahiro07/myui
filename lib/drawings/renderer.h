#pragma once
#include "../core/bridge-types.h"
#include "./drawing-types.h"
#include <cstdint>
#include <memory>

namespace myui::internal {

class Renderer : public DrawingContext {
public:
  virtual ~Renderer() = default;

  virtual bool resize(int width, int height) = 0;
  virtual void beginFrame(uint32_t clearColor = 0x00000000) = 0;
  virtual void endFrame() = 0;
  virtual const ImageData &getImageData() const = 0;
};

std::unique_ptr<Renderer> createBlend2dRenderer();

} // namespace myui::internal