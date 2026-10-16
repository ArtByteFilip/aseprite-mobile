// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/ios/event_queue.h"

#include "os/ios/ios_app.h"
#include "os/ios/window.h"

#include <algorithm>

namespace os {

void EventQueueIOS::queueEvent(const Event& ev)
{
  m_events.push(ev);
  ios_wakeup();
}

void EventQueueIOS::getEvent(Event& ev, double timeout)
{
  ev.setWindow(nullptr);

  // Process pending UIKit events without waiting
  ios_process_events(0.0);

  if (m_events.try_pop(ev))
    return;

  double wait = (timeout == kWithoutTimeout ? -1.0 : std::max(0.0, timeout));

  // Pending touch timers (long-press detection) limit the wait
  if (WindowIOS* window = WindowIOS::instance()) {
    const int timerMs = window->msecsToNextTimer();
    if (timerMs >= 0) {
      const double timerSecs = timerMs / 1000.0;
      wait = (wait < 0.0 ? timerSecs : std::min(wait, timerSecs));
    }
  }

  if (wait != 0.0)
    ios_process_events(wait);

  if (WindowIOS* window = WindowIOS::instance())
    window->processTimers();

  if (!m_events.try_pop(ev))
    ev.setType(Event::None);
}

void EventQueueIOS::clearEvents()
{
  m_events.clear();
}

} // namespace os
