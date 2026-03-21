#pragma once
#include "../core/bridge-types.h"
#include "../core/ui-frame-driver.h"
#include "../drawings/renderer.h"
#include "../editor-frame/editor-frame.h"
#include <functional>

namespace myui {

class EditorIntegration {
  std::unique_ptr<IEditorFrame> editorFrame;
  std::unique_ptr<Renderer> renderer;
  std::unique_ptr<UiFrameDriver> frameDriver;

public:
  void attachToParent(void *parent) {
    if (editorFrame)
      editorFrame->attachToParent(parent);
  }
  void removeFromParent() {
    if (editorFrame)
      editorFrame->removeFromParent();
  }
  void setBounds(int x, int y, int width, int height) {
    if (editorFrame)
      editorFrame->setBounds(x, y, width, height);
  }

  void setup(internal::UiProgramFn renderFn) {
    editorFrame =
        std::unique_ptr<myui::IEditorFrame>(myui::createEditorFrame());
    renderer = createBlend2dRenderer();
    auto dc = static_cast<DrawingContext *>(renderer.get());
    frameDriver = std::make_unique<UiFrameDriver>(*dc);

    editorFrame->setRenderCallback([this, renderFn](int w, int h) {
      if (!renderer || !frameDriver)
        return;
      renderer->resize(w, h);
      renderer->beginFrame(0x00000000);
      frameDriver->runFrame(renderFn, w, h);
      renderer->endFrame();
      editorFrame->setImageData(renderer->getImageData());
    });
    editorFrame->subscribePointer([this](const internal::PointerEvent &e) {
      if (frameDriver)
        frameDriver->handlePointerEventInput(e);
    });
  }
  void teardown() {
    if (editorFrame) {
      editorFrame->clearRenderCallback();
      editorFrame->unsubscribePointer();
      editorFrame.reset();
    }
    frameDriver.reset();
    renderer.reset();
  }
};

} // namespace myui