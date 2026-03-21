#pragma once
#include "../core/bridge-types.h"
#include "../core/ui-actor.h"
#include "../core/ui-frame-driver.h"
#include "../drawings/renderer.h"
#include "../infrastructure/editor-frame/editor-frame.h"
#include "../infrastructure/window/window.h"

namespace myui {

class MyuiApplication {
public:
  MyuiApplication() {};
  virtual void run(std::function<void(UiActor &, int, int)> renderFn) {
    auto window = internal::createWindow();
    auto editorFrame = internal::createEditorFrame();
    editorFrame->attachToParent(window->getRootViewHandle());
    auto renderer = internal::createBlend2dRenderer();
    auto dc = static_cast<DrawingContext *>(renderer.get());
    UiFrameDriver frameDriver{*dc};
    editorFrame->subscribePointer([&](const internal::PointerEvent &e) {
      frameDriver.handlePointerEventInput(e);
    });

    editorFrame->setRenderCallback([&](int w, int h) {
      renderer->resize(w, h);
      renderer->beginFrame(0x00000000);
      frameDriver.runFrame(renderFn, w, h);
      renderer->endFrame();
      editorFrame->setImageData(renderer->getImageData());
    });
    window->runEventLoop();
    editorFrame->clearRenderCallback();
    editorFrame->unsubscribePointer();
  }
};

} // namespace myui
