// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_EVENT_QUEUE_INCLUDED
#define OS_IOS_EVENT_QUEUE_INCLUDED
#pragma once

#include "base/concurrent_queue.h"
#include "os/event.h"
#include "os/event_queue.h"

namespace os {

class EventQueueIOS : public EventQueue {
public:
  void queueEvent(const Event& ev) override;
  void getEvent(Event& ev, double timeout) override;
  void clearEvents() override;

private:
  base::concurrent_queue<Event> m_events;
};

using EventQueueImpl = EventQueueIOS;

} // namespace os

#endif
