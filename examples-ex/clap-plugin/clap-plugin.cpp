#include <myui/editor.h>

using namespace myui;

// this is a pseudo code integrated into clap plugin

struct clap_window_t {
  void *win;
};

void renderRoot(UiActor &ui, int w, int h) {
  ui.box(w, h).draw([](auto &dc) {});
}

// clap plugin usage (wrapper for clap_plugin_gui_t)
class ClapController {
  std::unique_ptr<myui::IEditorFrame> editorFrame;
  std::unique_ptr<myui::Renderer> renderer;
  std::unique_ptr<myui::UiFrameDriver> frameDriver;

  bool guiCreate() {
    editorFrame =
        std::unique_ptr<myui::IEditorFrame>(myui::createEditorFrame());
    renderer = createBlend2dRenderer();
    auto dc = static_cast<DrawingContext *>(renderer.get());
    frameDriver = std::make_unique<UiFrameDriver>(*dc);

    editorFrame->setRenderCallback([this](int w, int h) {
      if (!renderer || !frameDriver)
        return;
      renderer->resize(w, h);
      renderer->beginFrame(0x00000000);
      frameDriver->runFrame(renderRoot, w, h);
      renderer->endFrame();
      editorFrame->setImageData(renderer->getImageData());
    });
    editorFrame->subscribePointer([this](const PointerEvent &e) {
      if (frameDriver)
        frameDriver->handlePointerEventInput(e);
    });
    return true;
  }
  void guiDestroy() {
    if (editorFrame) {
      editorFrame->clearRenderCallback();
      editorFrame->unsubscribePointer();
      editorFrame.reset();
    }
    frameDriver.reset();
    renderer.reset();
  }
  bool guiSetParent(const clap_window_t *window) {
    if (!editorFrame)
      return false;
    editorFrame->attachToParent(window->win);
    return true;
  }
  bool guiSetSize(uint32_t width, uint32_t height) {
    if (!editorFrame)
      return false;
    editorFrame->setBounds(0, 0, width, height);
    return true;
  }
  bool guiShow() { return true; }
  bool guiHide() { return true; }
};