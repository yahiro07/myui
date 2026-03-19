#import <Cocoa/Cocoa.h>

#include "window-floor.h"

#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <functional>
#include <memory>

using namespace myui;

class CocoaWindowFloor;

@interface CocoaWindowFloorView : NSView

- (instancetype)initWithFloor:(CocoaWindowFloor *)floor;

@end

@interface CocoaWindowFloorDelegate
    : NSObject <NSWindowDelegate, NSApplicationDelegate>

- (instancetype)initWithFloor:(CocoaWindowFloor *)floor;

@end

class CocoaWindowFloor : public WindowFloor {
private:
  NSWindow *window = nil;
  CocoaWindowFloorView *view = nil;
  CocoaWindowFloorDelegate *delegate = nil;
  NSTimer *displayTimer = nil;

  std::function<void(const PointerEvent &)> pointerListenerFn;
  std::function<void(int width, int height)> renderCallback;

  ImageData currentImageData{};
  bool hasImageData = false;

  int width = 800;
  int height = 600;
  int buttonFlags = 0;
  bool running = false;

  void requestRedraw() {
    if (view != nil) {
      [view setNeedsDisplay:YES];
    }
  }

public:
  ~CocoaWindowFloor() override { finalize(); }

  void setImageData(const ImageData &imageData) override {
    currentImageData = imageData;
    hasImageData =
        (currentImageData.buffer != nullptr && currentImageData.width > 0 &&
         currentImageData.height > 0 && currentImageData.strideBytes > 0);
  }

  void setRenderCallback(
      std::function<void(int width, int height)> callback) override {
    renderCallback = std::move(callback);
  }

  void clearRenderCallback() override { renderCallback = nullptr; }

  void subscribePointer(
      std::function<void(const PointerEvent &)> callback) override {
    pointerListenerFn = std::move(callback);
  }

  void unsubscribePointer() override { pointerListenerFn = nullptr; }

  void updateButtonFlag(NSInteger buttonNumber, bool pressed) {
    if (buttonNumber < 0 || buttonNumber > 2) {
      return;
    }

    const int flag = 1 << buttonNumber;
    if (pressed) {
      buttonFlags |= flag;
    } else {
      buttonFlags &= ~flag;
    }
  }

  void dispatchPointerEvent(NSEvent *event, PointerEventType type) {
    if (pointerListenerFn == nullptr || view == nil) {
      return;
    }

    NSPoint pointInView = [view convertPoint:event.locationInWindow
                                    fromView:nil];
    const NSRect bounds = view.bounds;
    const int x = (int)pointInView.x;
    // const int y = (int)(bounds.size.height - pointInView.y);
    const int y = (int)pointInView.y;
    pointerListenerFn({type, x, y, buttonFlags});
  }

  void handleMouseDown(NSEvent *event) {
    updateButtonFlag(event.buttonNumber, true);
    dispatchPointerEvent(event, PointerEventType::Down);
    requestRedraw();
  }

  void handleMouseDragged(NSEvent *event) {
    dispatchPointerEvent(event, PointerEventType::Move);
    requestRedraw();
  }

  void handleMouseUp(NSEvent *event) {
    updateButtonFlag(event.buttonNumber, false);
    dispatchPointerEvent(event, PointerEventType::Up);
    requestRedraw();
  }

  bool initialize() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    delegate = [[CocoaWindowFloorDelegate alloc] initWithFloor:this];
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

    [window setTitle:@"Blend2D test"];
    [window center];
    [window setAcceptsMouseMovedEvents:YES];

    view = [[CocoaWindowFloorView alloc] initWithFloor:this];
    if (view == nil) {
      finalize();
      return false;
    }

    [window setContentView:view];
    [window setInitialFirstResponder:view];
    [window makeFirstResponder:view];
    [window setDelegate:delegate];

    __weak CocoaWindowFloorView *weakView = view;
    displayTimer = [NSTimer
        scheduledTimerWithTimeInterval:(1.0 / 60.0)
                               repeats:YES
                                 block:^(__unused NSTimer *timer) {
                                   CocoaWindowFloorView *strongView = weakView;
                                   if (strongView != nil) {
                                     [strongView setNeedsDisplay:YES];
                                   }
                                 }];

    [window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    return true;
  }

  void renderInView(NSView *targetView) {
    const NSRect bounds = targetView.bounds;
    const int targetWidth = (int)NSWidth(bounds);
    const int targetHeight = (int)NSHeight(bounds);

    width = targetWidth;
    height = targetHeight;
    if (renderCallback) {
      renderCallback(targetWidth, targetHeight);
    }

    if (!hasImageData) {
      return;
    }

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (colorSpace == nullptr) {
      return;
    }

    CGContextRef cgContext = [[NSGraphicsContext currentContext] CGContext];
    const CGBitmapInfo bitmapInfo =
        kCGBitmapByteOrder32Little |
        static_cast<CGBitmapInfo>(kCGImageAlphaPremultipliedFirst);

    CGDataProviderRef provider = CGDataProviderCreateWithData(
        nullptr, currentImageData.buffer,
        (size_t)currentImageData.height * (size_t)currentImageData.strideBytes,
        nullptr);
    if (provider == nullptr) {
      CGColorSpaceRelease(colorSpace);
      return;
    }

    CGImageRef image =
        CGImageCreate(currentImageData.width, currentImageData.height, 8, 32,
                      currentImageData.strideBytes, colorSpace, bitmapInfo,
                      provider, nullptr, false, kCGRenderingIntentDefault);

    if (image != nullptr) {
      CGContextSaveGState(cgContext);
      CGContextSetInterpolationQuality(cgContext, kCGInterpolationNone);
      CGContextTranslateCTM(cgContext, 0.0, bounds.size.height);
      CGContextScaleCTM(cgContext, 1.0, -1.0);
      CGContextDrawImage(cgContext,
                         CGRectMake(0.0, 0.0, (CGFloat)currentImageData.width,
                                    (CGFloat)currentImageData.height),
                         image);
      CGContextRestoreGState(cgContext);
      CGImageRelease(image);
    }

    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
  }

  void stop() {
    if (!running) {
      return;
    }
    running = false;
    [NSApp stop:nil];
    requestRedraw();
  }

  void finalize() {
    if (displayTimer != nil) {
      [displayTimer invalidate];
      displayTimer = nil;
    }

    if (window != nil) {
      [window setDelegate:nil];
      [window orderOut:nil];
      window = nil;
    }

    view = nil;

    if (delegate != nil && NSApp.delegate == delegate) {
      [NSApp setDelegate:nil];
    }
    delegate = nil;
  }

  void run() override {
    if (!initialize()) {
      printf("Failed to initialize the window system.\n");
      return;
    }

    running = true;
    [NSApp run];
    finalize();
  }
};

@implementation CocoaWindowFloorView {
  CocoaWindowFloor *_floor;
}

- (instancetype)initWithFloor:(CocoaWindowFloor *)floor {
  self = [super initWithFrame:NSMakeRect(0, 0, 800, 600)];
  if (self != nil) {
    _floor = floor;
    [self setWantsLayer:YES];
  }
  return self;
}

- (BOOL)isFlipped {
  return YES;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
  [super drawRect:dirtyRect];
  if (_floor != nullptr) {
    _floor->renderInView(self);
  }
}

- (void)mouseDown:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDown(event);
  }
}

- (void)mouseDragged:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDragged(event);
  }
}

- (void)mouseUp:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseUp(event);
  }
}

- (void)rightMouseDown:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDown(event);
  }
}

- (void)rightMouseDragged:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDragged(event);
  }
}

- (void)rightMouseUp:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseUp(event);
  }
}

- (void)otherMouseDown:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDown(event);
  }
}

- (void)otherMouseDragged:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDragged(event);
  }
}

- (void)otherMouseUp:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseUp(event);
  }
}

- (void)mouseMoved:(NSEvent *)event {
  if (_floor != nullptr) {
    _floor->handleMouseDragged(event);
  }
}

@end

@implementation CocoaWindowFloorDelegate {
  CocoaWindowFloor *_floor;
}

- (instancetype)initWithFloor:(CocoaWindowFloor *)floor {
  self = [super init];
  if (self != nil) {
    _floor = floor;
  }
  return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:
    (NSApplication *)sender {
  return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
  if (_floor != nullptr) {
    _floor->stop();
  }
}

@end

WindowFloor *myui::createWindowFloor() { return new CocoaWindowFloor(); }
