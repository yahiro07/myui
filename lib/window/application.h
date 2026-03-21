#include "../core/drawings/renderer.h"
#include "../core/ui-actor.h"
#include "../core/ui-frame-driver.h"
#include "../editor-frame/editor-frame.h"
#include "window.h"

namespace myui {

class MyuiApplication {
public:
  MyuiApplication() {};
  virtual void run(std::function<void(UiActor &, int, int)> renderFn) {
    auto window = createWindow();
    auto editorFrame = createEditorFrame();
    editorFrame->attachToParent(window->getRootViewHandle());
    auto renderer = createBlend2dRenderer();
    auto dc = static_cast<DrawingContext *>(renderer.get());
    UiFrameDriver frameDriver{*dc};
    editorFrame->subscribePointer(
        [&](const PointerEvent &e) { frameDriver.handlePointerEventInput(e); });

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
