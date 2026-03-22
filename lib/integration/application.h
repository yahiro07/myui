#pragma once
#include "../core/bridge-types.h"
#include "../core/ui-actor.h"
#include "../infrastructure/editor-frame/editor-frame.h"
#include "../infrastructure/window/window.h"
#include "editor-integration.h"

namespace myui {

class MyuiApplication {
private:
  using IWindow = internal::IWindow;
  using IEditorFrame = internal::IEditorFrame;
  using Renderer = internal::Renderer;
  using PointerEvent = internal::PointerEvent;

private:
  std::unique_ptr<IWindow> window;
  std::unique_ptr<EditorIntegration> editorIntegration;

public:
  MyuiApplication() {
    window = internal::createWindow();
    editorIntegration = std::make_unique<EditorIntegration>();
    editorIntegration->attachToParent(window->getRootViewHandle());
  };
  ~MyuiApplication() {
    if (editorIntegration) {
      editorIntegration->removeFromParent();
      editorIntegration->teardown();
      editorIntegration.reset();
    }
    if (window) {
      window.reset();
    }
  }
  virtual void run(std::function<void(UiActor &, int, int)> renderFn) {
    if ((window && editorIntegration)) {
      editorIntegration->setup(renderFn);
      window->runEventLoop();
    }
  }
};

} // namespace myui
