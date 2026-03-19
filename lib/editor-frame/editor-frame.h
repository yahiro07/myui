#pragma once

#include "../core/bridge-types.h"
#include <functional>
#include <memory>

namespace myui {

class IEditorFrame {
public:
  virtual ~IEditorFrame() = default;
  virtual void attachToParent(void *parent) = 0;
  virtual void removeFromParent() = 0;
  virtual void setBounds(int x, int y, int width, int height) = 0;

  virtual void
  setRenderCallback(std::function<void(int width, int height)> callback) = 0;
  virtual void clearRenderCallback() = 0;

  virtual void
  subscribePointer(std::function<void(const PointerEvent &)> callback) = 0;
  virtual void unsubscribePointer() = 0;

  virtual void setImageData(const ImageData &imageData) = 0;
};

std::unique_ptr<IEditorFrame> createEditorFrame(); // cocoa/windows/x11

} // namespace myui