// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/ios/window.h"

#include "base/string.h"
#include "os/ios/ios_app.h"
#include "os/ios/screen.h"
#include "os/ios/system.h"
#include "os/ios/view.h"
#include "os/mobile.h"
#include "os/system.h"
#include "os/window_spec.h"

#include <algorithm>
#include <cmath>

namespace os {

namespace {

// Double-click timeout and distance (in points)
constexpr base::tick_t kDoubleClickMsecs = 400;
constexpr float kDoubleClickSlopPt = 24.0f;

// The on-screen keyboard is displayed only if a text field is focused
// in this period of time after a tap/click
constexpr base::tick_t kTextInputAfterTapMsecs = 500;

float distance(const gfx::PointF& a, const gfx::PointF& b)
{
  return std::hypot(a.x - b.x, a.y - b.y);
}

} // anonymous namespace

WindowIOS* WindowIOS::g_instance = nullptr;

WindowIOS::WindowIOS(const WindowSpec& spec) : m_scale(std::max(1, spec.scale()))
{
  if (!g_instance)
    g_instance = this;

  m_size = ios_view_pixel_size();
  if (m_size.w <= 0 || m_size.h <= 0)
    m_size = spec.contentRect().size();
  if (m_size.w <= 0 || m_size.h <= 0)
    m_size = gfx::Size(2048, 1536);
}

WindowIOS::~WindowIOS()
{
  if (g_instance == this)
    g_instance = nullptr;
}

// static
WindowIOS* WindowIOS::instance()
{
  return g_instance;
}

os::ScreenRef WindowIOS::screen() const
{
  return make_ref<ScreenIOS>();
}

os::ColorSpaceRef WindowIOS::colorSpace() const
{
  if (auto defaultCS = os::System::instance()->windowsColorSpace())
    return defaultCS;
  return System::instance()->makeColorSpace(gfx::ColorSpace::MakeSRGB());
}

Window::NativeHandle WindowIOS::nativeHandle() const
{
  return ios_main_view();
}

void WindowIOS::setScale(int scale)
{
  scale = std::max(1, scale);
  if (m_scale == scale)
    return;
  m_scale = scale;
  onResize(m_size);
}

void WindowIOS::invalidateRegion(const gfx::Region& rgn)
{
  onPaint();
}

void WindowIOS::setTextInput(bool state, const gfx::Point& screenCaretPos)
{
  if (state) {
    // Show the on-screen keyboard only when the text field was
    // focused by the user (e.g. tapping it), not when a dialog is
    // opened and its first text field is focused automatically.
    if (base::current_tick() - m_lastPointerTick > kTextInputAfterTapMsecs)
      return;
    if (screenCaretPos != gfx::Point(0, 0) &&
        std::abs(screenCaretPos.y - m_lastPointerScreenPos.y) > 24 * m_scale)
      return;
  }
  show_soft_keyboard(state);
}

void WindowIOS::onViewResized()
{
  const gfx::Size newSize = ios_view_pixel_size();
  if (newSize.w > 0 && newSize.h > 0 && newSize != m_size) {
    m_size = newSize;
    onResize(m_size);
  }
  else {
    onPaint();
  }
}

gfx::Point WindowIOS::toSurface(const gfx::PointF& pt) const
{
  return gfx::Point(int(pt.x) / m_scale, int(pt.y) / m_scale);
}

KeyModifiers WindowIOS::modifiers() const
{
  return KeyModifiers(int(m_keyModifiers) | int(virtual_modifiers()));
}

void WindowIOS::queueMouseEvent(Event::Type type,
                                const gfx::PointF& pos,
                                Event::MouseButton button,
                                PointerType pointerType,
                                float pressure)
{
  Event ev;
  ev.setType(type);
  ev.setPosition(toSurface(pos));
  ev.setModifiers(modifiers());
  ev.setButton(button);
  ev.setPointerType(pointerType);
  ev.setPressure(pressure);

  if (type == Event::MouseUp || type == Event::MouseDown) {
    m_lastPointerTick = base::current_tick();
    m_lastPointerScreenPos = gfx::Point(int(pos.x), int(pos.y));
  }

  // Double-click detection
  if (type == Event::MouseDown) {
    if (m_doubleClickButton == button &&
        base::current_tick() - m_doubleClickTick < kDoubleClickMsecs &&
        distance(pos, m_doubleClickPos) < touchDp(kDoubleClickSlopPt)) {
      ev.setType(Event::MouseDoubleClick);
      m_doubleClickButton = Event::NoneButton;
    }
    else {
      m_doubleClickButton = button;
      m_doubleClickTick = base::current_tick();
      m_doubleClickPos = pos;
    }
  }

  m_lastMousePos = ev.position();
  queueEvent(ev);
}

//////////////////////////////////////////////////////////////////////
// Fingers

void WindowIOS::onFingerDown(long id, const gfx::PointF& pos)
{
  m_touch.fingerDown(int(id), pos);
}

void WindowIOS::onFingerMove(long id, const gfx::PointF& pos)
{
  m_touch.fingerMove(int(id), pos);
}

void WindowIOS::onFingerMoveBatchEnd()
{
  m_touch.endOfMoveBatch();
}

void WindowIOS::onFingerUp(long id, const gfx::PointF& pos)
{
  m_touch.fingerUp(int(id), pos);
}

void WindowIOS::onFingersCancelled()
{
  m_touch.cancel();
}

//////////////////////////////////////////////////////////////////////
// Apple Pencil

void WindowIOS::onPencil(Event::Type type, const gfx::PointF& pos, float pressure, bool eraser)
{
  const PointerType pointerType = (eraser ? PointerType::Eraser : PointerType::Pen);
  switch (type) {
    case Event::MouseDown: {
      Event::MouseButton button = Event::LeftButton;
      if (consume_right_click_mode())
        button = Event::RightButton;
      m_pointerButton = button;
      queueMouseEvent(Event::MouseEnter, pos, Event::NoneButton, pointerType);
      queueMouseEvent(Event::MouseMove, pos, Event::NoneButton, pointerType, pressure);
      queueMouseEvent(Event::MouseDown, pos, button, pointerType, pressure);
      break;
    }
    case Event::MouseMove:
      queueMouseEvent(Event::MouseMove, pos, Event::NoneButton, pointerType, pressure);
      break;
    case Event::MouseUp:
      if (m_pointerButton != Event::NoneButton) {
        queueMouseEvent(Event::MouseUp, pos, m_pointerButton, pointerType);
        m_pointerButton = Event::NoneButton;
      }
      break;
    default: queueMouseEvent(type, pos, Event::NoneButton, pointerType); break;
  }
}

//////////////////////////////////////////////////////////////////////
// Mouse/trackpad (indirect pointer)

void WindowIOS::onPointer(Event::Type type, const gfx::PointF& pos, Event::MouseButton button)
{
  if (type == Event::MouseDown && button == Event::LeftButton && consume_right_click_mode())
    button = Event::RightButton;
  if (type == Event::MouseDown)
    m_pointerButton = button;
  else if (type == Event::MouseUp) {
    button = m_pointerButton;
    m_pointerButton = Event::NoneButton;
  }
  queueMouseEvent(type, pos, button, PointerType::Mouse);
}

void WindowIOS::onScroll(const gfx::PointF& pos, const gfx::PointF& delta)
{
  gfx::PointF d(delta.x / m_scale + m_scrollRemainder.x, delta.y / m_scale + m_scrollRemainder.y);
  const int dx = int(d.x);
  const int dy = int(d.y);
  m_scrollRemainder = gfx::PointF(d.x - dx, d.y - dy);
  if (dx == 0 && dy == 0)
    return;

  Event ev;
  ev.setType(Event::MouseWheel);
  ev.setPosition(toSurface(pos));
  ev.setModifiers(modifiers());
  ev.setPointerType(PointerType::Touchpad);
  ev.setWheelDelta(gfx::Point(-dx, -dy));
  ev.setPreciseWheel(true);
  queueEvent(ev);
}

//////////////////////////////////////////////////////////////////////
// Keyboard

void WindowIOS::onKey(bool down,
                      KeyScancode scancode,
                      int unicode,
                      KeyModifiers mods,
                      bool repeat)
{
  m_keyModifiers = mods;
  if (scancode != kKeyNil)
    SystemIOS::setKeyPressed(scancode, down);

  Event ev;
  ev.setType(down ? Event::KeyDown : Event::KeyUp);
  ev.setScancode(scancode);
  ev.setModifiers(modifiers());
  ev.setRepeat(repeat ? 1 : 0);
  // Don't generate characters when Cmd/Ctrl are pressed (shortcuts)
  if (down && (mods & (kKeyCtrlModifier | kKeyCmdModifier)) == 0 && unicode >= 32 &&
      unicode != 127)
    ev.setUnicodeChar(unicode);
  else
    ev.setUnicodeChar(0);
  queueEvent(ev);
}

void WindowIOS::onTextInput(const std::string& utf8)
{
  const std::wstring wide = base::from_utf8(utf8);
  for (const wchar_t chr : wide) {
    Event ev;
    ev.setScancode(kKeyNil);
    ev.setModifiers(kKeyNoneModifier);
    ev.setUnicodeChar(base::codepoint_t(chr));
    if (chr == '\n' || chr == '\r') {
      ev.setScancode(kKeyEnter);
      ev.setUnicodeChar(0);
    }
    else if (chr == ' ') {
      ev.setScancode(kKeySpace);
    }
    ev.setType(Event::KeyDown);
    queueEvent(ev);
    ev.setType(Event::KeyUp);
    ev.setUnicodeChar(0);
    queueEvent(ev);
  }
}

void WindowIOS::onBackspace()
{
  Event ev;
  ev.setScancode(kKeyBackspace);
  ev.setModifiers(kKeyNoneModifier);
  ev.setUnicodeChar(0);
  ev.setType(Event::KeyDown);
  queueEvent(ev);
  ev.setType(Event::KeyUp);
  queueEvent(ev);
}

void WindowIOS::releaseAllKeys()
{
  m_keyModifiers = kKeyNoneModifier;
  SystemIOS::resetAllKeys();
}

//////////////////////////////////////////////////////////////////////
// TouchGestures::Delegate impl

void WindowIOS::onTouchMouseEvent(Event::Type type,
                                  const gfx::PointF& pos,
                                  Event::MouseButton button)
{
  queueMouseEvent(type, pos, button, PointerType::Touch);
}

void WindowIOS::onTouchScroll(const gfx::PointF& pos, const gfx::Point& delta)
{
  Event ev;
  ev.setType(Event::MouseWheel);
  ev.setPosition(toSurface(pos));
  ev.setModifiers(modifiers());
  ev.setPointerType(PointerType::Touch);
  ev.setWheelDelta(delta);
  ev.setPreciseWheel(true);
  queueEvent(ev);
}

void WindowIOS::onTouchMagnify(const gfx::PointF& pos, float magnification)
{
  Event ev;
  ev.setType(Event::TouchMagnify);
  ev.setPosition(toSurface(pos));
  ev.setModifiers(modifiers());
  ev.setPointerType(PointerType::Touch);
  ev.setMagnification(magnification);
  queueEvent(ev);
}

void WindowIOS::onTouchGesture(TouchGestures::Gesture gesture)
{
  call_touch_gesture_handler(gesture == TouchGestures::Gesture::TwoFingerTap ?
                               TouchGesture::TwoFingerTap :
                               TouchGesture::ThreeFingerTap);
}

void WindowIOS::onTouchLongPress()
{
  ios_haptic_feedback();
}

bool WindowIOS::onTouchConsumeRightClickMode()
{
  return consume_right_click_mode();
}

float WindowIOS::touchDp(float v) const
{
  // 1 UIKit point ~= 1 dp
  return v * ios_content_scale();
}

} // namespace os
