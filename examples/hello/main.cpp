#include "../../lib/window/application.h"

using namespace myui;

int main(int argc, char *argv[]) {
  MyuiApplication app;
  app.run([](UiActor &ui, int w, int h) {
    ui.beginFrame();
    auto root = ui.rootBox(w, h).hCenter();
    root.sub([&ui] {
      ui.box(400, 300).draw([&ui](DrawingContext &dc) {
        dc.fillRect(0, 0, 400, 300, 0xff0000ff);
      });
    });
    ui.endFrame();
  });
  return 0;
}