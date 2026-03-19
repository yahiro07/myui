#pragma once

#include "../core/bridge-types.h"
#include <functional>
#include <memory>

namespace myui {

class IWindow {
public:
  virtual ~IWindow() = default;
  virtual void *getRootViewHandle() = 0;
  virtual void runEventLoop() = 0;
};

std::unique_ptr<IWindow> createWindow(); // cocoa/windows/x11

} // namespace myui