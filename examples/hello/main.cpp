#include <briq/application.h>

using namespace briq;

int main(int argc, char *argv[]) {
  BriqApplication app;
  app.loadFont("mainFont", "examples/fonts/Nurom-Bold.ttf");
  app.run([](UiActor &ui, int w, int h) {
    auto root = ui.rootBox(w, h).hCenter();
    root.sub([&] {
      ui.box(400, 300).draw([&](DrawingContext &dc) {
        dc.fillRect(0, 0, 400, 300, 0xff0000ff);
        dc.drawText("Hello World", 200, 150, "mainFont", 32, 0xffffffff, true);
      });
    });
  });
  return 0;
}