#include "../../lib/core/renderer.h"
#include "../../lib/core/ui-actor.h"
#include "../../lib/window/window-floor.h"
#include <algorithm>
#include <cstdio>
#include <functional>
#include <memory>

namespace dev0 {

using namespace myui;

class ParametersBridge {
public:
  ParametersBridge() {}
  ParametersBridge(const ParametersBridge &) = delete;
  ParametersBridge &operator=(const ParametersBridge &) = delete;

private:
  // create 3 parameters for debug
  float params[3] = {0.5f, 0.5f, 0.5f};

public:
  float get(int id) { return params[id]; }
  void beginEdit(int id) { printf("Begin edit: %d\n", id); }
  void performEdit(int id, float value) {
    printf("Perform Edit: %d, %f\n", id, value);
    params[id] = value;
  }
  void endEdit(int id) { printf("End edit: %d\n", id); }
};

class AppModel {
public:
  ParametersBridge &parametersBridge;

  AppModel(ParametersBridge &parametersBridge)
      : parametersBridge(parametersBridge) {}
  AppModel(const AppModel &) = delete;
  AppModel &operator=(const AppModel &) = delete;

  bool panelVisible = false;
};

void addKnobA(UiActor &ui, AppModel &model, int paramId, int color) {

  ui.box(100, 100).drawC([&model, paramId, color](auto &dc, auto &input) {
    auto &params = model.parametersBridge;
    auto hit = sqrt(input.x * input.x + input.y * input.y) < 50;

    auto bl = dc.devGetBlend2dContext();
    dc.strokeCircle(0, 0, 50, color);
    if (hit && input.hold) {
      dc.fillCircle(0, 0, 50, 0xff0000ff);
    }
    auto value = params.get(paramId);
    auto angle = (value * 2 - 1) * 135;
    bl.rotate(angle * 3.14159 / 180);
    dc.fillRect(-3, -50, 6, 25, 0xffff8800);
    if (hit && input.hold) {
      auto newValue = std::clamp(value - input.deltaY * 0.01f, 0.0f, 1.0f);
      params.performEdit(paramId, newValue);
    }
  });
}

void addKnobB(UiActor &ui, ParametersBridge &params) {
  ui.box(100, 100).draw([&params](auto &dc, auto &input) {
    auto cx = 50;
    auto cy = 50;
    auto r = 50;
    auto hit = sqrt((input.x - cx) * (input.x - cx) +
                    (input.y - cy) * (input.y - cy)) < r;
    dc.strokeCircle(cx, cy, r, 0xffFF8800);
    if (hit && input.hold) {
      dc.fillCircle(cx, cy, r, 0xffFF8800);
    }
    auto value = params.get(0);
    auto pos = value * 100;
    dc.fillRect(50 - 5, pos - 5, 10, 10, 0xffff8800);
  });
}

class EditorView {
  WindowFloor &windowFloor;
  Renderer &renderer;
  UiActor &ui;
  AppModel appModel;

  void render(int w, int h) {
    ui.beginFrame();
    auto root = ui.rootBox(w, h).hCenter();
    root.draw([&](auto &dc) { dc.fillRect(0, 0, w, h, 0xff448800); });
    root.sub([&] {
      auto panel = ui.box(500, 300).hCenter(20);
      panel.draw([](auto &dc) { dc.fillRect(0, 0, 500, 300, 0xffaabbcc); });
      panel.sub([&] {
        addKnobA(ui, appModel, 0, 0xff0000ff);
        addKnobB(ui, appModel.parametersBridge);
        auto knobC = ui.box(100, 100);
        knobC.draw([&](auto &dc, auto &input) {
          dc.strokeCircle(50, 50, 50, 0xffFF8800);
          if (input.hold) {
            dc.fillCircle(50, 50, 50, 0xffFF8800);
          }
        });
        auto knobD = ui.box(100, 100);
        knobD.draw([&](auto &dc, InputState &input) {
          dc.strokeCircle(50, 50, 50, 0xffFF8800);
        });
      });
    });
    ui.endFrame();
  }

public:
  EditorView(WindowFloor &windowFloor, Renderer &renderer, UiActor &ui,
             ParametersBridge &parametersBridge)
      : windowFloor(windowFloor), renderer(renderer), ui(ui),
        appModel(parametersBridge) {}

  void setup() {
    windowFloor.setRenderCallback([this](int w, int h) {
      renderer.resize(w, h);
      renderer.beginFrame(0x00000000);
      render(w, h);
      renderer.endFrame();
      windowFloor.setImageData(renderer.getImageData());
      ui.updatePointerStateOnFrameEnd();
      ui.debugFirstFrame = false;
    });
    windowFloor.subscribePointer(
        [this](const PointerEvent &e) { ui.handlePointerEventInput(e); });
  }
  void tearDown() {
    windowFloor.clearRenderCallback();
    windowFloor.unsubscribePointer();
  }

  void run() { windowFloor.run(); }
};

void entry() {
  printf("dev0 entry\n");
  auto windowFloor = std::unique_ptr<WindowFloor>(createWindowFloor());
  auto renderer = createBlend2dRenderer();
  UiActor uiActor(*renderer);
  ParametersBridge parametersBridge;
  EditorView editorView{*windowFloor, *renderer, uiActor, parametersBridge};
  editorView.setup();
  editorView.run();
  editorView.tearDown();
}

} // namespace dev0