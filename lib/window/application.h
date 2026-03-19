#include "../core/renderer.h"
#include "../core/ui-actor.h"
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
    UiActor ui{*renderer};
    editorFrame->subscribePointer(
        [&ui](const PointerEvent &e) { ui.handlePointerEventInput(e); });
    editorFrame->setRenderCallback([&](int w, int h) {
      renderer->resize(w, h);
      renderer->beginFrame(0x00000000);
      renderFn(ui, w, h);
      renderer->endFrame();
      editorFrame->setImageData(renderer->getImageData());
      ui.updatePointerStateOnFrameEnd();
      ui.debugFirstFrame = false;
    });
    window->runEventLoop();
    editorFrame->clearRenderCallback();
    editorFrame->unsubscribePointer();
  }
};

} // namespace myui
