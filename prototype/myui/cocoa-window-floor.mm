#import <Cocoa/Cocoa.h>

#include "window-floor.h"

#include <CoreGraphics/CoreGraphics.h>
#include <blend2d/blend2d.h>

#include <cstdint>
#include <functional>
#include <memory>

using namespace myui;

class CocoaWindowFloor;

static BLRgba32 colorFromUint32(uint32_t color) { return BLRgba32(color); }

class Blend2DDrawingContext : public DrawingContext {
private:
  BLContext *context = nullptr;

public:
  void setContext(BLContext *nextContext) { context = nextContext; }

  void fillRect(int x, int y, int w, int h, uint32_t color) override {
    if (context == nullptr) {
      return;
    }
    context->set_comp_op(BL_COMP_OP_SRC_OVER);
    context->set_fill_style(colorFromUint32(color));
    context->fill_rect(BLRect(x, y, w, h));
  }

  void strokeRect(int x, int y, int w, int h, uint32_t color) override {
    if (context == nullptr) {
      return;
    }
    context->set_comp_op(BL_COMP_OP_SRC_OVER);
    context->set_stroke_style(colorFromUint32(color));
    context->set_stroke_width(1.0);
    context->stroke_rect(BLRect(x, y, w, h));
  }

  void fillCircle(int cx, int cy, int r, uint32_t color) override {
    if (context == nullptr) {
      return;
    }
    context->set_comp_op(BL_COMP_OP_SRC_OVER);
    context->set_fill_style(colorFromUint32(color));
    context->fill_circle(cx, cy, r);
  }

  void strokeCircle(int cx, int cy, int r, uint32_t color) override {
    if (context == nullptr) {
      return;
    }
    context->set_comp_op(BL_COMP_OP_SRC_OVER);
    context->set_stroke_style(colorFromUint32(color));
    context->set_stroke_width(1.0);
    context->stroke_circle(cx, cy, r);
  }

  BLContext &devGetBlend2dContext() override { return *context; }
};

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

  Blend2DDrawingContext drawingContext;
  BLImage surface;

  std::function<void(const PointerEvent &)> pointerListenerFn;
  std::function<void()> renderCallback;

  int width = 800;
  int height = 600;
  int buttonFlags = 0;
  bool running = false;

  bool ensureSurface(int nextWidth, int nextHeight) {
    if (nextWidth <= 0 || nextHeight <= 0) {
      return false;
    }
    if (surface.width() == nextWidth && surface.height() == nextHeight) {
      return true;
    }

    BLImage nextSurface;
    if (nextSurface.create(nextWidth, nextHeight, BL_FORMAT_PRGB32) !=
        BL_SUCCESS) {
      return false;
    }

    surface = std::move(nextSurface);
    width = nextWidth;
    height = nextHeight;
    return true;
  }

  void requestRedraw() {
    if (view != nil) {
      [view setNeedsDisplay:YES];
    }
  }

public:
  ~CocoaWindowFloor() override { finalize(); }

  DrawingContext &getDrawingContext() override { return drawingContext; }

  void setRenderCallback(std::function<void()> callback) override {
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
    if (!ensureSurface(targetWidth, targetHeight)) {
      return;
    }

    BLContext context(surface);
    context.set_comp_op(BL_COMP_OP_SRC_COPY);
    context.fill_all(BLRgba32(0, 0, 0, 0));

    drawingContext.setContext(&context);
    if (renderCallback) {
      renderCallback();
    }
    drawingContext.setContext(nullptr);

    context.end();

    BLImageData imageData{};
    if (surface.get_data(&imageData) != BL_SUCCESS) {
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
        nullptr, imageData.pixel_data, imageData.size.h * imageData.stride,
        nullptr);
    if (provider == nullptr) {
      CGColorSpaceRelease(colorSpace);
      return;
    }

    CGImageRef image = CGImageCreate(
        imageData.size.w, imageData.size.h, 8, 32, imageData.stride, colorSpace,
        bitmapInfo, provider, nullptr, false, kCGRenderingIntentDefault);

    if (image != nullptr) {
      CGContextSaveGState(cgContext);
      CGContextSetInterpolationQuality(cgContext, kCGInterpolationNone);
      CGContextTranslateCTM(cgContext, 0.0, bounds.size.height);
      CGContextScaleCTM(cgContext, 1.0, -1.0);
      CGContextDrawImage(cgContext,
                         CGRectMake(0.0, 0.0, (CGFloat)imageData.size.w,
                                    (CGFloat)imageData.size.h),
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

    drawingContext.setContext(nullptr);
    surface.reset();

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
