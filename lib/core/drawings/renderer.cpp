#include "renderer.h"
#include <blend2d/blend2d.h>

namespace myui {

static BLRgba32 colorFromUint32(uint32_t color) { return BLRgba32(color); }

class Blend2dRendererImpl : public Renderer {
private:
  BLImage surface;
  BLContext ctx;
  bool hasContext = false;

  ImageData imageData{};
  int width = 0;
  int height = 0;

public:
  bool resize(int nextWidth, int nextHeight) override {
    if (nextWidth <= 0 || nextHeight <= 0) {
      return false;
    }
    if (width == nextWidth && height == nextHeight && surface.width() != 0 &&
        surface.height() != 0) {
      return true;
    }

    BLImage nextSurface;
    if (nextSurface.create(nextWidth, nextHeight, BL_FORMAT_PRGB32) !=
        BL_SUCCESS) {
      return false;
    }
    surface = std::move(nextSurface);
    width = nextWidth;
    height = nextHeight;
    return true;
  }

  void beginFrame(uint32_t clearColor) override {
    if (surface.width() <= 0 || surface.height() <= 0) {
      return;
    }

    ctx = BLContext(surface);
    hasContext = true;
    ctx.set_comp_op(BL_COMP_OP_SRC_COPY);
    ctx.fill_all(colorFromUint32(clearColor));
  }

  void endFrame() override {
    if (!hasContext) {
      return;
    }

    ctx.end();
    hasContext = false;

    BLImageData blImageData{};
    if (surface.get_data(&blImageData) != BL_SUCCESS) {
      imageData = {};
      return;
    }
    imageData.buffer = static_cast<const uint8_t *>(blImageData.pixel_data);
    imageData.width = blImageData.size.w;
    imageData.height = blImageData.size.h;
    imageData.strideBytes = blImageData.stride;
  }

  void fillRect(int x, int y, int w, int h, uint32_t color) override {
    ctx.set_comp_op(BL_COMP_OP_SRC_OVER);
    ctx.set_fill_style(colorFromUint32(color));
    ctx.fill_rect(BLRect(x, y, w, h));
  }

  void strokeRect(int x, int y, int w, int h, uint32_t color) override {
    ctx.set_comp_op(BL_COMP_OP_SRC_OVER);
    ctx.set_stroke_style(colorFromUint32(color));
    ctx.set_stroke_width(1.0);
    ctx.stroke_rect(BLRect(x, y, w, h));
  }

  void fillCircle(int cx, int cy, int r, uint32_t color) override {
    if (!hasContext) {
      return;
    }
    ctx.set_comp_op(BL_COMP_OP_SRC_OVER);
    ctx.set_fill_style(colorFromUint32(color));
    ctx.fill_circle(cx, cy, r);
  }

  void strokeCircle(int cx, int cy, int r, uint32_t color) override {
    ctx.set_comp_op(BL_COMP_OP_SRC_OVER);
    ctx.set_stroke_style(colorFromUint32(color));
    ctx.set_stroke_width(1.0);
    ctx.stroke_circle(cx, cy, r);
  }

  void save() override { ctx.save(); }

  void restore() override { ctx.restore(); }

  void translate(float x, float y) override { ctx.translate(x, y); }

  void rotate(float degree) override { ctx.rotate(degree); }

  void scale(float sx, float sy) override { ctx.scale(sx, sy); }

  void *devGetBlend2dContext() override { return &ctx; }

  const ImageData &getImageData() const override { return imageData; }
};

std::unique_ptr<Renderer> createBlend2dRenderer() {
  return std::make_unique<Blend2dRendererImpl>();
}

} // namespace myui