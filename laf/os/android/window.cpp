// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/android/window.h"

#include "base/string.h"
#include "os/android/android_app.h"
#include "os/android/keys.h"
#include "os/android/screen.h"
#include "os/android/system.h"
#include "os/event_queue.h"
#include "os/system.h"
#include "os/window_spec.h"

#include <android/input.h>
#include <android/keycodes.h>
#include <android/native_window.h>

#include <algorithm>
#include <cmath>

namespace os {

namespace {

// Double-click timeout and distance (in dp)
constexpr base::tick_t kDoubleClickMsecs = 400;
constexpr float kDoubleClickSlopDp = 24.0f;

// The on-screen keyboard is displayed only if a text field is focused
// in this period of time after a tap/click
constexpr base::tick_t kTextInputAfterTapMsecs = 500;

float distance(const gfx::PointF& a, const gfx::PointF& b)
{
  return std::hypot(a.x - b.x, a.y - b.y);
}

} // anonymous namespace

WindowAndroid* WindowAndroid::g_instance = nullptr;

WindowAndroid::WindowAndroid(const WindowSpec& spec) : m_scale(std::max(1, spec.scale()))
{
  if (!g_instance)
    g_instance = this;

  m_nativeWindow = android_native_window();
  if (m_nativeWindow) {
    ANativeWindow_setBuffersGeometry(m_nativeWindow, 0, 0, WINDOW_FORMAT_RGBX_8888);
    m_size.w = ANativeWindow_getWidth(m_nativeWindow);
    m_size.h = ANativeWindow_getHeight(m_nativeWindow);
  }
  if (m_size.w <= 0 || m_size.h <= 0)
    m_size = spec.contentRect().size();
  if (m_size.w <= 0 || m_size.h <= 0)
    m_size = gfx::Size(1280, 720);
}

WindowAndroid::~WindowAndroid()
{
  if (g_instance == this)
    g_instance = nullptr;
}

// static
WindowAndroid* WindowAndroid::instance()
{
  return g_instance;
}

os::ScreenRef WindowAndroid::screen() const
{
  return make_ref<ScreenAndroid>();
}

os::ColorSpaceRef WindowAndroid::colorSpace() const
{
  if (auto defaultCS = os::System::instance()->windowsColorSpace())
    return defaultCS;
  return System::instance()->makeColorSpace(gfx::ColorSpace::MakeSRGB());
}

void WindowAndroid::setScale(int scale)
{
  scale = std::max(1, scale);
  if (m_scale == scale)
    return;
  m_scale = scale;
  onResize(m_size);
}

gfx::Rect WindowAndroid::frame() const
{
  return gfx::Rect(0, 0, m_size.w, m_size.h);
}

gfx::Rect WindowAndroid::contentRect() const
{
  return gfx::Rect(0, 0, m_size.w, m_size.h);
}

void WindowAndroid::invalidateRegion(const gfx::Region& rgn)
{
  if (!m_nativeWindow)
    return;

  gfx::Region realRgn;
  for (const gfx::Rect& rc : rgn)
    realRgn |= gfx::Region(
      gfx::Rect(rc.x * m_scale, rc.y * m_scale, rc.w * m_scale, rc.h * m_scale));

  onPaint(m_nativeWindow, realRgn);
}

void WindowAndroid::setTextInput(bool state, const gfx::Point& screenCaretPos)
{
  if (state) {
    // Show the on-screen keyboard only when the text field was
    // focused by the user (e.g. tapping it), not when a dialog is
    // opened and its first text field is focused automatically
    // (in that case the keyboard would cover the dialog).
    if (base::current_tick() - m_lastPointerUpTick > kTextInputAfterTapMsecs)
      return;
    // The tap must be near the text caret (same row)
    if (screenCaretPos != gfx::Point(0, 0) &&
        std::abs(screenCaretPos.y - m_lastPointerScreenPos.y) > 24 * m_scale)
      return;
  }
  show_soft_keyboard(state);
}

void WindowAndroid::onNativeWindowChanged(ANativeWindow* nativeWindow)
{
  m_nativeWindow = nativeWindow;
  if (m_nativeWindow) {
    ANativeWindow_setBuffersGeometry(m_nativeWindow, 0, 0, WINDOW_FORMAT_RGBX_8888);
    checkSize();
    redrawAll();
  }
}

void WindowAndroid::checkSize()
{
  if (!m_nativeWindow)
    return;

  const gfx::Size newSize(ANativeWindow_getWidth(m_nativeWindow),
                          ANativeWindow_getHeight(m_nativeWindow));
  if (newSize.w > 0 && newSize.h > 0 && newSize != m_size) {
    m_size = newSize;
    onResize(m_size);
  }
}

void WindowAndroid::redrawAll()
{
  if (m_nativeWindow)
    onPaint(m_nativeWindow, gfx::Region(gfx::Rect(m_size)));
}

float WindowAndroid::dp(float v) const
{
  return v * float(android_screen_dpi()) / 160.0f;
}

gfx::Point WindowAndroid::toSurface(const gfx::PointF& pt) const
{
  return gfx::Point(int(pt.x) / m_scale, int(pt.y) / m_scale);
}

KeyModifiers WindowAndroid::modifiers() const
{
  return KeyModifiers(int(m_keyModifiers) | int(virtual_modifiers()));
}

void WindowAndroid::queueMouseEvent(Event::Type type,
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
    m_lastPointerUpTick = base::current_tick();
    m_lastPointerScreenPos = gfx::Point(int(pos.x), int(pos.y));
  }

  // Double-click detection
  if (type == Event::MouseDown) {
    if (m_doubleClickButton == button &&
        base::current_tick() - m_doubleClickTick < kDoubleClickMsecs &&
        distance(pos, m_doubleClickPos) < dp(kDoubleClickSlopDp)) {
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

int32_t WindowAndroid::handleInputEvent(AInputEvent* event)
{
  switch (AInputEvent_getType(event)) {
    case AINPUT_EVENT_TYPE_KEY:    return handleKeyEvent(event);
    case AINPUT_EVENT_TYPE_MOTION: return handleMotionEvent(event);
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////
// Keyboard

int32_t WindowAndroid::handleKeyEvent(AInputEvent* event)
{
  const int32_t keyCode = AKeyEvent_getKeyCode(event);
  const int32_t action = AKeyEvent_getAction(event);
  const int32_t metaState = AKeyEvent_getMetaState(event);

  // Let the system handle volume/media/power keys
  switch (keyCode) {
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_VOLUME_MUTE:
    case AKEYCODE_POWER:
    case AKEYCODE_HOME:
    case AKEYCODE_APP_SWITCH:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MEDIA_PLAY:
    case AKEYCODE_MEDIA_PAUSE:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:  return 0;
  }

  // Multiple characters (e.g. from IMEs) are handled in Java through
  // the InputConnection (see AsepriteActivity.java).
  if (action == AKEY_EVENT_ACTION_MULTIPLE)
    return 0;

  m_keyModifiers = android_meta_state_to_modifiers(metaState);

  const KeyScancode scancode = android_keycode_to_scancode(keyCode);
  if (scancode != kKeyNil)
    SystemAndroid::setKeyPressed(scancode, action == AKEY_EVENT_ACTION_DOWN);

  Event ev;
  ev.setType(action == AKEY_EVENT_ACTION_DOWN ? Event::KeyDown : Event::KeyUp);
  ev.setScancode(scancode);
  ev.setModifiers(modifiers());
  ev.setRepeat(AKeyEvent_getRepeatCount(event));

  if (action == AKEY_EVENT_ACTION_DOWN) {
    int unicode = 0;
    // Don't generate characters when Ctrl/Alt are pressed (shortcuts)
    if ((metaState & (AMETA_CTRL_ON | AMETA_ALT_ON | AMETA_META_ON)) == 0)
      unicode = android_get_unicode_char(keyCode, metaState);
    if (unicode < 32 || unicode == 127) // Control characters
      unicode = 0;
    ev.setUnicodeChar(unicode);
  }

  queueEvent(ev);
  return 1;
}

void WindowAndroid::queueTextInput(const std::string& utf8)
{
  const std::wstring wide = base::from_utf8(utf8);
  for (const wchar_t chr : wide) {
    Event ev;
    ev.setScancode(kKeyNil);
    ev.setModifiers(kKeyNoneModifier);
    ev.setUnicodeChar(base::codepoint_t(chr));

    // Map some special characters to their scancodes (so the
    // Entry/TextEdit widgets can process them correctly)
    if (chr == '\n') {
      ev.setScancode(kKeyEnter);
      ev.setUnicodeChar(0);
    }
    else if (chr == '\b') {
      ev.setScancode(kKeyBackspace);
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

void WindowAndroid::releaseAllKeys()
{
  m_keyModifiers = kKeyNoneModifier;
  SystemAndroid::resetAllKeys();
}

//////////////////////////////////////////////////////////////////////
// Mouse/stylus/touch

int32_t WindowAndroid::handleMotionEvent(AInputEvent* event)
{
  const int32_t action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
  const int32_t source = AInputEvent_getSource(event);
  const int32_t toolType = AMotionEvent_getToolType(event, 0);

  m_keyModifiers = android_meta_state_to_modifiers(AMotionEvent_getMetaState(event));

  // Mouse wheel/scroll
  if (action == AMOTION_EVENT_ACTION_SCROLL) {
    const float vscroll = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, 0);
    const float hscroll = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HSCROLL, 0);
    Event ev;
    ev.setType(Event::MouseWheel);
    ev.setPosition(
      toSurface(gfx::PointF(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0))));
    ev.setModifiers(modifiers());
    ev.setPointerType(PointerType::Mouse);
    ev.setWheelDelta(gfx::Point(hscroll > 0.0f ? 1 : (hscroll < 0.0f ? -1 : 0),
                                vscroll > 0.0f ? -1 : (vscroll < 0.0f ? 1 : 0)));
    queueEvent(ev);
    return 1;
  }

  if (toolType == AMOTION_EVENT_TOOL_TYPE_FINGER &&
      (source & AINPUT_SOURCE_TOUCHSCREEN) == AINPUT_SOURCE_TOUCHSCREEN) {
    return handleTouchEvent(event, action);
  }

  if (toolType == AMOTION_EVENT_TOOL_TYPE_PALM)
    return 1; // Ignore palms

  return handleStylusOrMouseEvent(event, action, toolType);
}

int32_t WindowAndroid::handleStylusOrMouseEvent(AInputEvent* event,
                                                int32_t action,
                                                int32_t toolType)
{
  const gfx::PointF pos(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
  const int32_t buttonState = AMotionEvent_getButtonState(event);

  PointerType pointerType = PointerType::Mouse;
  if (toolType == AMOTION_EVENT_TOOL_TYPE_STYLUS)
    pointerType = PointerType::Pen;
  else if (toolType == AMOTION_EVENT_TOOL_TYPE_ERASER)
    pointerType = PointerType::Eraser;
  else if (toolType == AMOTION_EVENT_TOOL_TYPE_FINGER)
    pointerType = PointerType::Touch;

  const bool isPen = (pointerType == PointerType::Pen || pointerType == PointerType::Eraser);
  const float pressure = (isPen ? std::clamp(AMotionEvent_getPressure(event, 0), 0.0f, 1.0f) :
                                  0.0f);

  switch (action) {
    case AMOTION_EVENT_ACTION_HOVER_ENTER:
    case AMOTION_EVENT_ACTION_HOVER_MOVE:
      queueMouseEvent(Event::MouseMove, pos, Event::NoneButton, pointerType);
      break;

    case AMOTION_EVENT_ACTION_HOVER_EXIT: break;

    case AMOTION_EVENT_ACTION_DOWN: {
      Event::MouseButton button = Event::LeftButton;
      if (pointerType == PointerType::Mouse) {
        if (buttonState & AMOTION_EVENT_BUTTON_SECONDARY)
          button = Event::RightButton;
        else if (buttonState & AMOTION_EVENT_BUTTON_TERTIARY)
          button = Event::MiddleButton;
      }
      else if (buttonState &
               (AMOTION_EVENT_BUTTON_STYLUS_PRIMARY | AMOTION_EVENT_BUTTON_SECONDARY)) {
        // Stylus side button acts as the right mouse button
        button = Event::RightButton;
      }
      if (consume_right_click_mode())
        button = Event::RightButton;

      m_pointerButton = button;
      m_mouseButtonState = buttonState;
      queueMouseEvent(Event::MouseMove, pos, Event::NoneButton, pointerType, pressure);
      queueMouseEvent(Event::MouseDown, pos, button, pointerType, pressure);
      break;
    }

    case AMOTION_EVENT_ACTION_MOVE: {
      // Use historical samples for smoother strokes
      const size_t n = AMotionEvent_getHistorySize(event);
      for (size_t i = 0; i < n; ++i) {
        const gfx::PointF hpos(AMotionEvent_getHistoricalX(event, 0, i),
                               AMotionEvent_getHistoricalY(event, 0, i));
        const float hpressure =
          (isPen ? std::clamp(AMotionEvent_getHistoricalPressure(event, 0, i), 0.0f, 1.0f) :
                   0.0f);
        queueMouseEvent(Event::MouseMove, hpos, Event::NoneButton, pointerType, hpressure);
      }
      queueMouseEvent(Event::MouseMove, pos, Event::NoneButton, pointerType, pressure);
      break;
    }

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_CANCEL:
      if (m_pointerButton != Event::NoneButton) {
        queueMouseEvent(Event::MouseUp, pos, m_pointerButton, pointerType);
        m_pointerButton = Event::NoneButton;
      }
      m_mouseButtonState = 0;
      break;

    case AMOTION_EVENT_ACTION_BUTTON_PRESS:
    case AMOTION_EVENT_ACTION_BUTTON_RELEASE: {
      // Extra mouse buttons pressed while another button is already
      // pressed (the first button generates ACTION_DOWN/UP).
      const int32_t changed = buttonState ^ m_mouseButtonState;
      m_mouseButtonState = buttonState;
      Event::MouseButton button = Event::NoneButton;
      if (changed & AMOTION_EVENT_BUTTON_SECONDARY)
        button = Event::RightButton;
      else if (changed & AMOTION_EVENT_BUTTON_TERTIARY)
        button = Event::MiddleButton;
      else if (changed & AMOTION_EVENT_BUTTON_BACK)
        button = Event::X1Button;
      else if (changed & AMOTION_EVENT_BUTTON_FORWARD)
        button = Event::X2Button;
      if (button != Event::NoneButton && button != m_pointerButton) {
        queueMouseEvent(action == AMOTION_EVENT_ACTION_BUTTON_PRESS ? Event::MouseDown :
                                                                      Event::MouseUp,
                        pos,
                        button,
                        pointerType,
                        pressure);
      }
      break;
    }
  }
  return 1;
}

int32_t WindowAndroid::handleTouchEvent(AInputEvent* event, int32_t action)
{
  switch (action) {
    case AMOTION_EVENT_ACTION_DOWN:
    case AMOTION_EVENT_ACTION_POINTER_DOWN: {
      const size_t index = (action == AMOTION_EVENT_ACTION_DOWN ?
                              0 :
                              (AMotionEvent_getAction(event) &
                               AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
      m_touch.fingerDown(
        AMotionEvent_getPointerId(event, index),
        gfx::PointF(AMotionEvent_getX(event, index), AMotionEvent_getY(event, index)));
      break;
    }

    case AMOTION_EVENT_ACTION_MOVE: {
      const size_t n = AMotionEvent_getPointerCount(event);
      const size_t h = AMotionEvent_getHistorySize(event);
      // Use historical samples for smoother strokes
      for (size_t j = 0; j < h; ++j) {
        for (size_t i = 0; i < n; ++i) {
          m_touch.fingerMove(AMotionEvent_getPointerId(event, i),
                             gfx::PointF(AMotionEvent_getHistoricalX(event, i, j),
                                         AMotionEvent_getHistoricalY(event, i, j)));
        }
      }
      for (size_t i = 0; i < n; ++i) {
        m_touch.fingerMove(AMotionEvent_getPointerId(event, i),
                           gfx::PointF(AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)));
      }
      m_touch.endOfMoveBatch();
      break;
    }

    case AMOTION_EVENT_ACTION_UP:
    case AMOTION_EVENT_ACTION_POINTER_UP: {
      const size_t index = (action == AMOTION_EVENT_ACTION_UP ?
                              0 :
                              (AMotionEvent_getAction(event) &
                               AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
      m_touch.fingerUp(
        AMotionEvent_getPointerId(event, index),
        gfx::PointF(AMotionEvent_getX(event, index), AMotionEvent_getY(event, index)));
      break;
    }

    case AMOTION_EVENT_ACTION_CANCEL: m_touch.cancel(); break;
  }
  return 1;
}

//////////////////////////////////////////////////////////////////////
// TouchGestures::Delegate impl

void WindowAndroid::onTouchMouseEvent(Event::Type type,
                                      const gfx::PointF& pos,
                                      Event::MouseButton button)
{
  queueMouseEvent(type, pos, button, PointerType::Touch);
}

void WindowAndroid::onTouchScroll(const gfx::PointF& pos, const gfx::Point& delta)
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

void WindowAndroid::onTouchMagnify(const gfx::PointF& pos, float magnification)
{
  Event ev;
  ev.setType(Event::TouchMagnify);
  ev.setPosition(toSurface(pos));
  ev.setModifiers(modifiers());
  ev.setPointerType(PointerType::Touch);
  ev.setMagnification(magnification);
  queueEvent(ev);
}

void WindowAndroid::onTouchGesture(TouchGestures::Gesture gesture)
{
  call_touch_gesture_handler(gesture == TouchGestures::Gesture::TwoFingerTap ?
                               TouchGesture::TwoFingerTap :
                               TouchGesture::ThreeFingerTap);
}

void WindowAndroid::onTouchLongPress()
{
  android_haptic_feedback();
}

bool WindowAndroid::onTouchConsumeRightClickMode()
{
  return consume_right_click_mode();
}

float WindowAndroid::touchDp(float v) const
{
  return dp(v);
}

int WindowAndroid::msecsToNextTimer() const
{
  return m_touch.msecsToNextTimer();
}

void WindowAndroid::processTimers()
{
  m_touch.processTimers();
}

} // namespace os
