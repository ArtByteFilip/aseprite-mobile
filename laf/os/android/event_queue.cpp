// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/android/event_queue.h"

#include "base/time.h"
#include "os/android/android_app.h"
#include "os/android/window.h"

#include <android/looper.h>
#include <android_native_app_glue.h>

#include <algorithm>
#include <cmath>

namespace os {

void EventQueueAndroid::queueEvent(const Event& ev)
{
  m_events.push(ev);

  // Wake up the native thread if it's waiting for events in
  // ALooper_pollOnce()
  if (android_app* app = android_get_app()) {
    if (app->looper)
      ALooper_wake(app->looper);
  }
}

void EventQueueAndroid::getEvent(Event& ev, double timeout)
{
  ev.setWindow(nullptr);

  // Process all pending Android events without waiting
  android_process_events(0);

  if (m_events.try_pop(ev))
    return;

  // Calculate how much time we can wait for new events
  int waitMs;
  if (timeout == kWithoutTimeout)
    waitMs = -1;
  else if (timeout > 0.0)
    waitMs = int(std::ceil(timeout * 1000.0));
  else
    waitMs = 0;

  // Pending touch timers (e.g. long-press detection) limit the wait
  if (WindowAndroid* window = WindowAndroid::instance()) {
    const int timerMs = window->msecsToNextTimer();
    if (timerMs >= 0)
      waitMs = (waitMs < 0 ? timerMs : std::min(waitMs, timerMs));
  }

  if (waitMs != 0)
    android_process_events(waitMs);

  if (WindowAndroid* window = WindowAndroid::instance())
    window->processTimers();

  if (!m_events.try_pop(ev))
    ev.setType(Event::None);
}

void EventQueueAndroid::clearEvents()
{
  m_events.clear();
}

} // namespace os
