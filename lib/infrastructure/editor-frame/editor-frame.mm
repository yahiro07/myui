#import <Cocoa/Cocoa.h>

#include "editor-frame.h"

#include <CoreGraphics/CoreGraphics.h>

#include <cstdint>
#include <functional>
#include <memory>

using namespace briq;
using namespace briq::internal;

class CocoaEditorFrame;

@interface CocoaEditorFrameView : NSView

- (instancetype)initWithEditorFrame:(CocoaEditorFrame *)editorFrame;

@end

class CocoaEditorFrame : public IEditorFrame {
private:
  NSView *parent = nil;
  CocoaEditorFrameView *view = nil;
  NSTimer *displayTimer = nil;

  std::function<void(int width, int height)> renderCallback;
  std::function<void(const PointerEvent &)> pointerListenerFn;

  ImageData currentImageData{};
  bool hasImageData = false;

  int buttonFlags = 0;

  void requestRedraw() {
    if (view != nil) {
      [view setNeedsDisplay:YES];
    }
  }

  void startDisplayTimer() {
    if (displayTimer != nil) {
      return;
    }

    __weak CocoaEditorFrameView *weakView = view;
    displayTimer = [NSTimer
        scheduledTimerWithTimeInterval:(1.0 / 60.0)
                               repeats:YES
                                 block:^(__unused NSTimer *timer) {
                                   CocoaEditorFrameView *strongView = weakView;
                                   if (strongView != nil) {
                                     [strongView setNeedsDisplay:YES];
                                   }
                                 }];
  }

  void stopDisplayTimer() {
    if (displayTimer != nil) {
      [displayTimer invalidate];
      displayTimer = nil;
    }
  }

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
    const int x = (int)pointInView.x;
    const int y = (int)pointInView.y;
    pointerListenerFn({type, x, y, buttonFlags});
  }

public:
  ~CocoaEditorFrame() override { removeFromParent(); }

  void attachToParent(void *parentHandle) override {
    parent = (__bridge NSView *)parentHandle;
    if (parent == nil) {
      return;
    }

    if (view == nil) {
      view = [[CocoaEditorFrameView alloc] initWithEditorFrame:this];
      if (view == nil) {
        parent = nil;
        return;
      }
      view.frame = parent.bounds;
      view.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    }

    if (view.superview != parent) {
      [parent addSubview:view];
    }

    startDisplayTimer();
    requestRedraw();
  }

  void removeFromParent() override {
    stopDisplayTimer();

    if (view != nil && view.superview != nil) {
      [view removeFromSuperview];
    }
    parent = nil;
  }

  void setBounds(int x, int y, int width, int height) override {
    if (view == nil) {
      return;
    }
    view.frame = NSMakeRect(x, y, width, height);
    requestRedraw();
  }

  void setRenderCallback(
      std::function<void(int width, int height)> callback) override {
    renderCallback = std::move(callback);
    requestRedraw();
  }

  void clearRenderCallback() override { renderCallback = nullptr; }

  void subscribePointer(
      std::function<void(const PointerEvent &)> callback) override {
    pointerListenerFn = std::move(callback);
  }

  void unsubscribePointer() override { pointerListenerFn = nullptr; }

  void setImageData(const ImageData &imageData) override {
    currentImageData = imageData;
    hasImageData =
        (currentImageData.buffer != nullptr && currentImageData.width > 0 &&
         currentImageData.height > 0 && currentImageData.strideBytes > 0);
    requestRedraw();
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

  void renderInView(NSView *targetView) {
    if (targetView == nil) {
      return;
    }

    const NSRect bounds = targetView.bounds;
    const int targetWidth = (int)NSWidth(bounds);
    const int targetHeight = (int)NSHeight(bounds);
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
};

@implementation CocoaEditorFrameView {
  CocoaEditorFrame *_editorFrame;
  NSTrackingArea *_trackingArea;
}

- (instancetype)initWithEditorFrame:(CocoaEditorFrame *)editorFrame {
  self = [super initWithFrame:NSMakeRect(0, 0, 800, 600)];
  if (self != nil) {
    _editorFrame = editorFrame;
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

- (void)updateTrackingAreas {
  [super updateTrackingAreas];

  if (_trackingArea != nil) {
    [self removeTrackingArea:_trackingArea];
    _trackingArea = nil;
  }

  NSTrackingAreaOptions options = NSTrackingMouseMoved |
                                  NSTrackingActiveInKeyWindow |
                                  NSTrackingInVisibleRect;
  _trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                               options:options
                                                 owner:self
                                              userInfo:nil];
  [self addTrackingArea:_trackingArea];
}

- (void)drawRect:(NSRect)dirtyRect {
  [super drawRect:dirtyRect];
  if (_editorFrame != nullptr) {
    _editorFrame->renderInView(self);
  }
}

- (void)mouseDown:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDown(event);
  }
}

- (void)mouseDragged:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDragged(event);
  }
}

- (void)mouseUp:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseUp(event);
  }
}

- (void)rightMouseDown:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDown(event);
  }
}

- (void)rightMouseDragged:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDragged(event);
  }
}

- (void)rightMouseUp:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseUp(event);
  }
}

- (void)otherMouseDown:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDown(event);
  }
}

- (void)otherMouseDragged:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDragged(event);
  }
}

- (void)otherMouseUp:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseUp(event);
  }
}

- (void)mouseMoved:(NSEvent *)event {
  if (_editorFrame != nullptr) {
    _editorFrame->handleMouseDragged(event);
  }
}

@end

std::unique_ptr<IEditorFrame> briq::internal::createEditorFrame() {
  return std::make_unique<CocoaEditorFrame>();
}
