#include "../core/ui-actor.h"
#include "editor-frame.h"
#include "window.h"

namespace myui {

class MyuiApplication {
public:
  MyuiApplication();
  virtual void run(std::function<void(UiActor &)> renderFn) {
    auto window = createWindow();
    auto editorFrame = createEditorFrame();
    editorFrame->attachToParent(window->getRootViewHandle());
    UiActor ui;
    auto dc = createDrawingContext();
    ui.setDrawingContext(dc.get());
    editorFrame->subscribePointer(
        [&](auto &e) { ui.handlePointerEventInput(e); });
    editorFrame->setRenderCallback([&] {
      renderFn(ui);
      editorFrame->setImageData(dc->getImageData());
    });
    window->runEventLoop();
    // cleanup things here if needed
  }
};

} // namespace myui
