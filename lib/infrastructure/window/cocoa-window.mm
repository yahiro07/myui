#import <Cocoa/Cocoa.h>

#include "window.h"

#include <cstdio>
#include <memory>

using namespace myui::internal;

class CocoaWindow;

@interface CocoaWindowDelegate
    : NSObject <NSWindowDelegate, NSApplicationDelegate>

- (instancetype)initWithWindow:(CocoaWindow *)window;

@end

class CocoaWindow : public IWindow {
private:
  NSWindow *window = nil;
  CocoaWindowDelegate *delegate = nil;
  bool running = false;

  int width = 800;
  int height = 600;

  bool initialize() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    delegate = [[CocoaWindowDelegate alloc] initWithWindow:this];
    [NSApp setDelegate:delegate];

    const NSRect frame = NSMakeRect(0, 0, width, height);
    window =
        [[NSWindow alloc] initWithContentRect:frame
                                    styleMask:(NSWindowStyleMaskTitled |
                                               NSWindowStyleMaskClosable |
                                               NSWindowStyleMaskMiniaturizable |
                                               NSWindowStyleMaskResizable)
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    if (window == nil) {
      finalize();
      return false;
    }

    [window setTitle:@"myui"];
    [window center];
    [window setAcceptsMouseMovedEvents:YES];
    [window setDelegate:delegate];

    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];

    return true;
  }

  bool ensureInitialized() {
    if (window != nil) {
      return true;
    }
    return initialize();
  }

  void finalize() {
    if (window != nil) {
      [window setDelegate:nil];
      [window orderOut:nil];
      window = nil;
    }

    if (delegate != nil && NSApp.delegate == delegate) {
      [NSApp setDelegate:nil];
    }
    delegate = nil;
  }

public:
  ~CocoaWindow() override { finalize(); }

  void *getRootViewHandle() override {
    if (!ensureInitialized()) {
      return nullptr;
    }
    return (__bridge void *)window.contentView;
  }

  void stop() {
    if (!running) {
      return;
    }
    running = false;
    [NSApp stop:nil];
  }

  void runEventLoop() override {
    if (!ensureInitialized()) {
      printf("Failed to initialize the window system.\n");
      return;
    }

    running = true;
    [NSApp run];
    running = false;

    finalize();
  }
};

@implementation CocoaWindowDelegate {
  CocoaWindow *_window;
}

- (instancetype)initWithWindow:(CocoaWindow *)window {
  self = [super init];
  if (self != nil) {
    _window = window;
  }
  return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
  if (_window != nullptr) {
    _window->stop();
  }
}

@end

std::unique_ptr<IWindow> myui::internal::createWindow() {
  return std::make_unique<CocoaWindow>();
}
