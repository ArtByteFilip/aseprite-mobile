// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_WINDOW_INCLUDED
#define OS_IOS_WINDOW_INCLUDED
#pragma once

#include "base/time.h"
#include "gfx/point.h"
#include "gfx/region.h"
#include "gfx/size.h"
#include "os/common/touch_gestures.h"
#include "os/event.h"
#include "os/native_cursor.h"
#include "os/screen.h"
#include "os/window.h"

#include <string>

namespace os {

class WindowSpec;

// iPadOS only supports one full-screen window (the AsepriteView
// inside the main UIWindow).
class WindowIOS : public Window,
                  public TouchGestures::Delegate {
public:
  WindowIOS(const WindowSpec& spec);
  ~WindowIOS();

  // Returns the only window that receives the UIKit input events.
  static WindowIOS* instance();

  os::ScreenRef screen() const override;
  os::ColorSpaceRef colorSpace() const override;

  int scale() const override { return m_scale; }
  void setScale(int scale) override;

  bool isVisible() const override { return true; }
  void setVisible(bool visible) override {}

  void activate() override {}
  void maximize() override {}
  void minimize() override {}
  bool isMaximized() const override { return true; }
  bool isMinimized() const override { return false; }
  bool isTransparent() const override { return false; }

  bool isFullscreen() const override { return true; }
  void setFullscreen(bool state) override {}

  void setTitle(const std::string& title) override { m_title = title; }
  std::string title() const override { return m_title; }

  gfx::Rect frame() const override { return gfx::Rect(m_size); }
  void setFrame(const gfx::Rect& bounds) override {}
  gfx::Rect contentRect() const override { return gfx::Rect(m_size); }
  gfx::Rect restoredFrame() const override { return frame(); }

  gfx::Size clientSize() const { return m_size; }

  void captureMouse() override {}
  void releaseMouse() override {}
  void setMousePosition(const gfx::Point& position) override {}
  void invalidateRegion(const gfx::Region& rgn) override;
  bool setCursor(NativeCursor cursor) override { return true; }
  bool setCursor(const CursorRef& cursor) override { return true; }

  void performWindowAction(WindowAction action, const Event* event) override {}

  NativeHandle nativeHandle() const override;

  void setTextInput(bool state, const gfx::Point& screenCaretPos = {});

  // Events from the AsepriteView (positions in real pixels)
  void onViewResized();
  void onFingerDown(long id, const gfx::PointF& pos);
  void onFingerMove(long id, const gfx::PointF& pos);
  void onFingerMoveBatchEnd();
  void onFingerUp(long id, const gfx::PointF& pos);
  void onFingersCancelled();
  void onPencil(Event::Type type, const gfx::PointF& pos, float pressure, bool eraser);
  void onPointer(Event::Type type, const gfx::PointF& pos, Event::MouseButton button);
  void onScroll(const gfx::PointF& pos, const gfx::PointF& delta);
  void onKey(bool down, KeyScancode scancode, int unicode, KeyModifiers modifiers, bool repeat);
  void onTextInput(const std::string& utf8);
  void onBackspace();
  void releaseAllKeys();

  // Touch timers (long-press detection)
  int msecsToNextTimer() const { return m_touch.msecsToNextTimer(); }
  void processTimers() { m_touch.processTimers(); }

  gfx::Point lastMousePos() const { return m_lastMousePos; }

  // TouchGestures::Delegate impl
  void onTouchMouseEvent(Event::Type type,
                         const gfx::PointF& pos,
                         Event::MouseButton button) override;
  void onTouchScroll(const gfx::PointF& pos, const gfx::Point& delta) override;
  void onTouchMagnify(const gfx::PointF& pos, float magnification) override;
  void onTouchGesture(TouchGestures::Gesture gesture) override;
  void onTouchLongPress() override;
  bool onTouchConsumeRightClickMode() override;
  float touchDp(float v) const override;
  int touchScale() const override { return m_scale; }

protected:
  // Paints the whole surface in the view
  virtual void onPaint() = 0;
  virtual void onResize(const gfx::Size& sz) = 0;

private:
  gfx::Point toSurface(const gfx::PointF& pt) const;
  KeyModifiers modifiers() const;
  void queueMouseEvent(Event::Type type,
                       const gfx::PointF& pos,
                       Event::MouseButton button,
                       PointerType pointerType,
                       float pressure = 0.0f);

  static WindowIOS* g_instance;

  gfx::Size m_size;
  int m_scale = 1;
  std::string m_title;
  gfx::Point m_lastMousePos;
  KeyModifiers m_keyModifiers = kKeyNoneModifier;
  TouchGestures m_touch{ this };

  // Double-click detection
  Event::MouseButton m_doubleClickButton = Event::NoneButton;
  base::tick_t m_doubleClickTick = 0;
  gfx::PointF m_doubleClickPos;

  // Pencil/pointer button pressed
  Event::MouseButton m_pointerButton = Event::NoneButton;

  // To show the on-screen keyboard only when the user taps a text field
  base::tick_t m_lastPointerTick = 0;
  gfx::Point m_lastPointerScreenPos;

  // Accumulated scroll (trackpad/mouse wheel)
  gfx::PointF m_scrollRemainder;
};

} // namespace os

#endif
