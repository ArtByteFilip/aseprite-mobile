// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_ANDROID_WINDOW_INCLUDED
#define OS_ANDROID_WINDOW_INCLUDED
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
#include <vector>

struct AInputEvent;
struct ANativeWindow;

namespace os {

class WindowSpec;

// Android only supports one full-screen window: the native window of
// the NativeActivity.
class WindowAndroid : public Window,
                      public TouchGestures::Delegate {
public:
  WindowAndroid(const WindowSpec& spec);
  ~WindowAndroid();

  // Returns the only window that receives the Android input events.
  static WindowAndroid* instance();

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

  gfx::Rect frame() const override;
  void setFrame(const gfx::Rect& bounds) override {}
  gfx::Rect contentRect() const override;
  gfx::Rect restoredFrame() const override { return frame(); }

  gfx::Size clientSize() const { return m_size; }

  void captureMouse() override {}
  void releaseMouse() override {}
  void setMousePosition(const gfx::Point& position) override {}
  void invalidateRegion(const gfx::Region& rgn) override;
  bool setCursor(NativeCursor cursor) override { return true; }
  bool setCursor(const CursorRef& cursor) override { return true; }

  void performWindowAction(WindowAction action, const Event* event) override {}

  NativeHandle nativeHandle() const override { return (NativeHandle)m_nativeWindow; }

  void setTextInput(bool state, const gfx::Point& screenCaretPos = {});

  // Called from the Android activity events
  void onNativeWindowChanged(ANativeWindow* nativeWindow);
  void checkSize();
  void redrawAll();
  int32_t handleInputEvent(AInputEvent* event);
  void queueTextInput(const std::string& utf8);
  void releaseAllKeys();

  // Touch timers (long-press detection)
  int msecsToNextTimer() const;
  void processTimers();

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
  // Draws the given rectangle (in real native window coordinates)
  // from the surface to the native window.
  virtual void onPaint(ANativeWindow* nativeWindow, const gfx::Region& rgn) = 0;
  virtual void onResize(const gfx::Size& sz) = 0;

private:
  int32_t handleKeyEvent(AInputEvent* event);
  int32_t handleMotionEvent(AInputEvent* event);
  int32_t handleTouchEvent(AInputEvent* event, int32_t action);
  int32_t handleStylusOrMouseEvent(AInputEvent* event, int32_t action, int32_t toolType);

  gfx::Point toSurface(const gfx::PointF& pt) const;
  KeyModifiers modifiers() const;
  void queueMouseEvent(Event::Type type,
                       const gfx::PointF& pos,
                       Event::MouseButton button,
                       PointerType pointerType,
                       float pressure = 0.0f);
  float dp(float v) const;

  static WindowAndroid* g_instance;

  ANativeWindow* m_nativeWindow = nullptr;
  gfx::Size m_size;
  int m_scale = 1;
  std::string m_title;
  gfx::Point m_lastMousePos;
  KeyModifiers m_keyModifiers = kKeyNoneModifier;

  // Touch gestures
  TouchGestures m_touch{ this };

  // Double-click detection
  Event::MouseButton m_doubleClickButton = Event::NoneButton;
  base::tick_t m_doubleClickTick = 0;
  gfx::PointF m_doubleClickPos;

  // Stylus/mouse state
  Event::MouseButton m_pointerButton = Event::NoneButton;
  int32_t m_mouseButtonState = 0;

  // Last time a mouse button was pressed/released (to show the
  // on-screen keyboard only when the user taps a text field)
  base::tick_t m_lastPointerUpTick = 0;
  gfx::Point m_lastPointerScreenPos;
};

} // namespace os

#endif
