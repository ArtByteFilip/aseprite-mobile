// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_ANDROID_EVENT_QUEUE_INCLUDED
#define OS_ANDROID_EVENT_QUEUE_INCLUDED
#pragma once

#include "base/concurrent_queue.h"
#include "os/event.h"
#include "os/event_queue.h"

#include <atomic>

namespace os {

class EventQueueAndroid : public EventQueue {
public:
  void queueEvent(const Event& ev) override;
  void getEvent(Event& ev, double timeout) override;
  void clearEvents() override;

  bool isEmpty() const { return m_events.empty(); }

private:
  base::concurrent_queue<Event> m_events;
};

using EventQueueImpl = EventQueueAndroid;

} // namespace os

#endif
