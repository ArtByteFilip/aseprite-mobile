// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/mobile.h"

#include "os/event.h"
#include "os/event_queue.h"

#include <atomic>

namespace os {

namespace {

TouchGestureHandler g_gestureHandler;
AppPauseHandler g_pauseHandler;
std::atomic<bool> g_rightClickMode(false);
std::atomic<int> g_virtualModifiers(0);

void queue_callback(std::function<void()>&& func)
{
  Event ev;
  ev.setType(Event::Callback);
  ev.setCallback(std::move(func));
  EventQueue::instance()->queueEvent(ev);
}

} // anonymous namespace

void set_touch_gesture_handler(TouchGestureHandler&& handler)
{
  g_gestureHandler = std::move(handler);
}

void call_touch_gesture_handler(TouchGesture gesture)
{
  if (!g_gestureHandler)
    return;

  // Execute the handler from the UI thread when the event is
  // processed (not in the middle of the input event processing).
  queue_callback([gesture] {
    if (g_gestureHandler)
      g_gestureHandler(gesture);
  });
}

void set_app_pause_handler(AppPauseHandler&& handler)
{
  g_pauseHandler = std::move(handler);
}

void call_app_pause_handler()
{
  if (!g_pauseHandler)
    return;
  queue_callback([] {
    if (g_pauseHandler)
      g_pauseHandler();
  });
}

void set_right_click_mode(bool state)
{
  g_rightClickMode = state;
}

bool right_click_mode()
{
  return g_rightClickMode;
}

bool consume_right_click_mode()
{
  return g_rightClickMode.exchange(false);
}

void set_virtual_modifiers(KeyModifiers modifiers)
{
  g_virtualModifiers = int(modifiers);
}

KeyModifiers virtual_modifiers()
{
  return KeyModifiers(g_virtualModifiers.load());
}

} // namespace os
