// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/ios/view.h"

#include "os/ios/ios_app.h"
#include "os/ios/keys.h"
#include "os/ios/window.h"

#include <QuartzCore/QuartzCore.h>

using namespace os;

namespace {

WindowIOS* laf_window()
{
  return WindowIOS::instance();
}

KeyModifiers modifiers_from_flags(UIKeyModifierFlags flags)
{
  int mods = kKeyNoneModifier;
  if (flags & UIKeyModifierShift)
    mods |= kKeyShiftModifier;
  if (flags & UIKeyModifierControl)
    mods |= kKeyCtrlModifier;
  if (flags & UIKeyModifierAlternate)
    mods |= kKeyAltModifier;
  if (flags & UIKeyModifierCommand)
    mods |= kKeyCmdModifier;
  return KeyModifiers(mods);
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////
// AsepriteKeyInputView

@implementation AsepriteKeyInputView

- (BOOL)canBecomeFirstResponder
{
  return YES;
}

- (BOOL)hasText
{
  return YES;
}

- (void)insertText:(NSString*)text
{
  if (WindowIOS* window = laf_window())
    window->onTextInput(std::string([text UTF8String]));
}

- (void)deleteBackward
{
  if (WindowIOS* window = laf_window())
    window->onBackspace();
}

- (UITextAutocorrectionType)autocorrectionType
{
  return UITextAutocorrectionTypeNo;
}

- (UITextAutocapitalizationType)autocapitalizationType
{
  return UITextAutocapitalizationTypeNone;
}

- (UITextSpellCheckingType)spellCheckingType
{
  return UITextSpellCheckingTypeNo;
}

- (UIReturnKeyType)returnKeyType
{
  return UIReturnKeyDone;
}

@end

//////////////////////////////////////////////////////////////////////
// AsepriteView

@implementation AsepriteView {
  CALayer* m_contentLayer;
}

- (instancetype)initWithFrame:(CGRect)frame
{
  self = [super initWithFrame:frame];
  if (self) {
    self.multipleTouchEnabled = YES;
    self.backgroundColor = [UIColor blackColor];

    m_contentLayer = [CALayer layer];
    m_contentLayer.magnificationFilter = kCAFilterNearest;
    m_contentLayer.minificationFilter = kCAFilterNearest;
    m_contentLayer.contentsGravity = kCAGravityResize;
    m_contentLayer.anchorPoint = CGPointMake(0, 0);
    [self.layer addSublayer:m_contentLayer];

    // Mouse/trackpad/Apple Pencil hover
    UIHoverGestureRecognizer* hover =
      [[UIHoverGestureRecognizer alloc] initWithTarget:self action:@selector(onHover:)];
    [self addGestureRecognizer:hover];

    // Mouse wheel/trackpad scroll (without touches)
    UIPanGestureRecognizer* scroll =
      [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onScroll:)];
    scroll.allowedScrollTypesMask = UIScrollTypeMaskAll;
    scroll.maximumNumberOfTouches = 0;
    [self addGestureRecognizer:scroll];
  }
  return self;
}

- (void)layoutSubviews
{
  [super layoutSubviews];
  if (WindowIOS* window = laf_window())
    window->onViewResized();
}

- (void)presentImage:(CGImageRef)image
          imageWidth:(int)width
         imageHeight:(int)height
         windowScale:(int)scale
{
  const CGFloat contentScale = self.contentScaleFactor;
  [CATransaction begin];
  [CATransaction setDisableActions:YES];
  // Each surface pixel is drawn as a "scale x scale" block of real
  // pixels.
  m_contentLayer.frame =
    CGRectMake(0, 0, width * scale / contentScale, height * scale / contentScale);
  m_contentLayer.contents = (__bridge id)image;
  [CATransaction commit];
}

- (gfx::PointF)pixelPosition:(CGPoint)pt
{
  const CGFloat s = self.contentScaleFactor;
  return gfx::PointF(pt.x * s, pt.y * s);
}

// Touches

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  for (UITouch* touch in touches) {
    const gfx::PointF pos = [self pixelPosition:[touch locationInView:self]];
    switch (touch.type) {
      case UITouchTypePencil: {
        const float pressure =
          (touch.maximumPossibleForce > 0 ? touch.force / touch.maximumPossibleForce : 1.0f);
        window->onPencil(os::Event::MouseDown, pos, pressure, false);
        break;
      }
      case UITouchTypeIndirectPointer: {
        os::Event::MouseButton button = os::Event::LeftButton;
        if (event.buttonMask & UIEventButtonMaskSecondary)
          button = os::Event::RightButton;
        window->onPointer(os::Event::MouseDown, pos, button);
        break;
      }
      default: window->onFingerDown(long((__bridge void*)touch), pos); break;
    }
  }
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  bool fingers = false;
  for (UITouch* touch in touches) {
    switch (touch.type) {
      case UITouchTypePencil: {
        // Use coalesced touches for smoother strokes
        NSArray<UITouch*>* samples = [event coalescedTouchesForTouch:touch];
        if (!samples)
          samples = @[ touch ];
        for (UITouch* sample in samples) {
          const float pressure = (sample.maximumPossibleForce > 0 ?
                                    sample.force / sample.maximumPossibleForce :
                                    1.0f);
          window->onPencil(os::Event::MouseMove,
                           [self pixelPosition:[sample locationInView:self]],
                           pressure,
                           false);
        }
        break;
      }
      case UITouchTypeIndirectPointer:
        window->onPointer(os::Event::MouseMove,
                          [self pixelPosition:[touch locationInView:self]],
                          os::Event::NoneButton);
        break;
      default: {
        NSArray<UITouch*>* samples = [event coalescedTouchesForTouch:touch];
        if (!samples)
          samples = @[ touch ];
        for (UITouch* sample in samples) {
          window->onFingerMove(long((__bridge void*)touch),
                               [self pixelPosition:[sample locationInView:self]]);
        }
        fingers = true;
        break;
      }
    }
  }
  if (fingers)
    window->onFingerMoveBatchEnd();
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  for (UITouch* touch in touches) {
    const gfx::PointF pos = [self pixelPosition:[touch locationInView:self]];
    switch (touch.type) {
      case UITouchTypePencil:
        window->onPencil(os::Event::MouseUp, pos, 0.0f, false);
        break;
      case UITouchTypeIndirectPointer:
        window->onPointer(os::Event::MouseUp, pos, os::Event::NoneButton);
        break;
      default: window->onFingerUp(long((__bridge void*)touch), pos); break;
    }
  }
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  bool fingers = false;
  for (UITouch* touch in touches) {
    const gfx::PointF pos = [self pixelPosition:[touch locationInView:self]];
    switch (touch.type) {
      case UITouchTypePencil:
        window->onPencil(os::Event::MouseUp, pos, 0.0f, false);
        break;
      case UITouchTypeIndirectPointer:
        window->onPointer(os::Event::MouseUp, pos, os::Event::NoneButton);
        break;
      default: fingers = true; break;
    }
  }
  if (fingers)
    window->onFingersCancelled();
}

// Hover (mouse, trackpad, Apple Pencil hover)

- (void)onHover:(UIHoverGestureRecognizer*)recognizer
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  const gfx::PointF pos = [self pixelPosition:[recognizer locationInView:self]];
  switch (recognizer.state) {
    case UIGestureRecognizerStateBegan:
      window->onPointer(os::Event::MouseEnter, pos, os::Event::NoneButton);
      window->onPointer(os::Event::MouseMove, pos, os::Event::NoneButton);
      break;
    case UIGestureRecognizerStateChanged:
      window->onPointer(os::Event::MouseMove, pos, os::Event::NoneButton);
      break;
    case UIGestureRecognizerStateEnded:
    case UIGestureRecognizerStateCancelled:
      window->onPointer(os::Event::MouseLeave, pos, os::Event::NoneButton);
      break;
    default: break;
  }
}

// Mouse wheel/trackpad scroll

- (void)onScroll:(UIPanGestureRecognizer*)recognizer
{
  WindowIOS* window = laf_window();
  if (!window)
    return;

  const CGPoint t = [recognizer translationInView:self];
  [recognizer setTranslation:CGPointZero inView:self];
  const CGFloat s = self.contentScaleFactor;
  window->onScroll([self pixelPosition:[recognizer locationInView:self]],
                   gfx::PointF(t.x * s, t.y * s));
}

@end

//////////////////////////////////////////////////////////////////////
// AsepriteViewController

@implementation AsepriteViewController

- (void)loadView
{
  self.asepriteView = [[AsepriteView alloc] initWithFrame:[UIScreen mainScreen].bounds];
  self.view = self.asepriteView;

  self.keyInputView = [[AsepriteKeyInputView alloc] initWithFrame:CGRectMake(0, 0, 1, 1)];
  self.keyInputView.hidden = NO;
  self.keyInputView.alpha = 0.0;
  [self.view addSubview:self.keyInputView];
}

- (void)viewDidAppear:(BOOL)animated
{
  [super viewDidAppear:animated];
  [self becomeFirstResponder];
}

- (BOOL)prefersStatusBarHidden
{
  return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden
{
  return YES;
}

- (UIRectEdge)preferredScreenEdgesDeferringSystemGestures
{
  return UIRectEdgeAll;
}

- (BOOL)canBecomeFirstResponder
{
  return YES;
}

- (void)showSoftKeyboard:(BOOL)state
{
  if (state) {
    [self.keyInputView becomeFirstResponder];
  }
  else if (self.keyInputView.isFirstResponder) {
    [self.keyInputView resignFirstResponder];
    [self becomeFirstResponder];
  }
}

// Hardware keyboard

- (void)handlePresses:(NSSet<UIPress*>*)presses down:(BOOL)down withEvent:(UIPressesEvent*)event
{
  WindowIOS* window = laf_window();
  for (UIPress* press in presses) {
    UIKey* key = press.key;
    if (!key || !window)
      continue;

    // When the on-screen keyboard is visible, the text is received
    // through AsepriteKeyInputView (UIKeyInput)
    if (self.keyInputView.isFirstResponder)
      continue;

    const KeyScancode scancode = ios_hid_usage_to_scancode(long(key.keyCode));
    int unicode = 0;
    NSString* chars = key.characters;
    if (chars.length == 1)
      unicode = [chars characterAtIndex:0];
    window->onKey(down, scancode, unicode, modifiers_from_flags(key.modifierFlags), false);
  }
}

- (void)pressesBegan:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
  [self handlePresses:presses down:YES withEvent:event];
  if (self.keyInputView.isFirstResponder)
    [super pressesBegan:presses withEvent:event];
}

- (void)pressesEnded:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
  [self handlePresses:presses down:NO withEvent:event];
  if (self.keyInputView.isFirstResponder)
    [super pressesEnded:presses withEvent:event];
}

- (void)pressesCancelled:(NSSet<UIPress*>*)presses withEvent:(UIPressesEvent*)event
{
  [self handlePresses:presses down:NO withEvent:event];
  if (self.keyInputView.isFirstResponder)
    [super pressesCancelled:presses withEvent:event];
}

@end
