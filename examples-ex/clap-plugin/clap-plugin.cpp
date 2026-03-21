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
  myui::EditorIntegration editorIntegration;

public:
  bool guiCreate() {
    editorIntegration.setup(renderRoot);
    return true;
  }
  void guiDestroy() { editorIntegration.teardown(); }
  bool guiSetParent(const clap_window_t *window) {
    editorIntegration.attachToParent(window->win);
    return true;
  }
  bool guiSetSize(uint32_t width, uint32_t height) {
    editorIntegration.setBounds(0, 0, width, height);
    return true;
  }
  bool guiShow() { return true; }
  bool guiHide() { return true; }
};