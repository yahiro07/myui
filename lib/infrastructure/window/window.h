#pragma once
#include <memory>

namespace briq::internal {

class IWindow {
public:
  virtual ~IWindow() = default;
  virtual void *getRootViewHandle() = 0;
  virtual void runEventLoop() = 0;
};

std::unique_ptr<IWindow> createWindow(); // cocoa/windows/x11

} // namespace briq::internal