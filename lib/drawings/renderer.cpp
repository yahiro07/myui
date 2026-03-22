#include "renderer.h"
#include <blend2d/blend2d.h>
#include <unordered_map>

namespace myui::internal {

static BLRgba32 colorFromUint32(uint32_t color) { return BLRgba32(color); }

class FontManager {
private:
  std::unordered_map<std::string, BLFontFace> fonts;

public:
  void loadFontFace(std::string fontKey, std::string fontFilePath) {
    BLFontFace face;
    BLResult result = face.create_from_file(fontFilePath.c_str());
    if (result != BL_SUCCESS) {
      printf("Failed to load a face (err=%u)\n", result);
      return;
    }
    fonts[fontKey] = face;
  }

  BLFontFace *getFontFace(std::string fontKey) {
    if (fonts.contains(fontKey)) {
      return &fonts.at(fontKey);
    }
    return nullptr;
  }
};

class Blend2dRendererImpl : public Renderer {
private:
  FontManager fontManager;
  BLImage surface;
  BLContext ctx;
  bool hasContext = false;

  ImageData imageData{};
  int width = 0;
  int height = 0;

public:
  BLContext *devGetBlend2dContext() override { return &ctx; }

  const ImageData &getImageData() const override { return imageData; }

  void loadFont(std::string fontKey, std::string fontFilePath) override {
    fontManager.loadFontFace(fontKey, fontFilePath);
  }

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

  void drawText(std::string text, int ox, int oy, std::string fontKey, int size,
                uint32_t color, bool centered) override {

    BLFontFace *face = fontManager.getFontFace(fontKey);
    if (!face)
      return;
    BLFont font;
    font.create_from_face(*face, size);

    auto blColor = colorFromUint32(color);
    BLFontMetrics fm = font.metrics();
    auto x = ox;
    auto y = oy + fm.ascent / 2;
    if (centered) {
      BLGlyphBuffer gb;
      gb.set_utf8_text(text.data(), text.size());
      font.shape(gb);
      BLTextMetrics tm;
      font.get_text_metrics(gb, tm);
      // x = ox - (tm.bounding_box.x1 - tm.bounding_box.x0) / 2;
      x = ox - tm.advance.x / 2;
      ctx.fill_glyph_run(BLPoint(x, y), font, gb.glyph_run(), blColor);
    } else {
      ctx.set_fill_style(blColor);
      ctx.fill_utf8_text(BLPoint(x, y), font, text.c_str());
    }
  }
};

std::unique_ptr<Renderer> createBlend2dRenderer() {
  return std::make_unique<Blend2dRendererImpl>();
}

} // namespace myui::internal