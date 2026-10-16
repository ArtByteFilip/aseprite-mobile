// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_COMMON_TOUCH_GESTURES_H_INCLUDED
#define OS_COMMON_TOUCH_GESTURES_H_INCLUDED
#pragma once

#include "base/time.h"
#include "gfx/point.h"
#include "os/event.h"

#include <vector>

namespace os {

// Converts finger touches (touch screens on mobile devices) into
// mouse-like events:
//
// * Tap: left click
// * Drag with one finger: left button drag
// * Long-press + release: right click
// * Two fingers: pan (precise mouse wheel) and pinch (TouchMagnify)
// * Two/three fingers tap: TouchGesture::TwoFingerTap/ThreeFingerTap
//
// All positions are in real window pixels.
class TouchGestures {
public:
  enum class Gesture {
    TwoFingerTap,
    ThreeFingerTap,
  };

  class Delegate {
  public:
    virtual ~Delegate() {}
    // Mouse event in the given position (MouseEnter/Leave/Move/Down/Up)
    virtual void onTouchMouseEvent(Event::Type type,
                                   const gfx::PointF& pos,
                                   Event::MouseButton button) = 0;
    // Scroll in surface pixels (already divided by the window scale)
    virtual void onTouchScroll(const gfx::PointF& pos, const gfx::Point& delta) = 0;
    virtual void onTouchMagnify(const gfx::PointF& pos, float magnification) = 0;
    virtual void onTouchGesture(Gesture gesture) = 0;
    // Called when a long-press is recognized (e.g. to vibrate)
    virtual void onTouchLongPress() = 0;
    // Returns true if the next touch must be a right click (and
    // resets that mode)
    virtual bool onTouchConsumeRightClickMode() = 0;
    // Density-independent pixels to real pixels
    virtual float touchDp(float v) const = 0;
    // Window scale (to convert real pixels into surface pixels)
    virtual int touchScale() const = 0;
  };

  TouchGestures(Delegate* delegate) : m_delegate(delegate) {}

  // Finger events (call fingerMove() for each historical/coalesced
  // sample, and then endOfMoveBatch() once).
  void fingerDown(int id, const gfx::PointF& pos);
  void fingerMove(int id, const gfx::PointF& pos);
  void endOfMoveBatch();
  void fingerUp(int id, const gfx::PointF& pos);
  void cancel();

  // Timers to detect long-press. msecsToNextTimer() returns -1 if
  // there is no pending timer.
  int msecsToNextTimer() const;
  void processTimers();

  bool isActive() const { return m_state != State::Idle; }

private:
  enum class State {
    Idle,     // No fingers on the screen
    Pending,  // One finger on the screen, no mouse button pressed yet
    Dragging, // One finger converted to mouse button down+move events
    Gesture,  // Two or more fingers (pan/zoom/undo/redo gestures)
    Finished, // Waiting all fingers to be released
  };

  struct Finger {
    int id;
    gfx::PointF pos;
  };

  Finger* findFinger(int id);
  gfx::PointF centroid() const;
  float spread() const;
  void startGesture();
  void queueGestureEvents();
  void mouse(Event::Type type, const gfx::PointF& pos, Event::MouseButton button);

  Delegate* m_delegate;
  State m_state = State::Idle;
  std::vector<Finger> m_fingers;
  int m_primaryId = -1;
  gfx::PointF m_startPos;
  gfx::PointF m_lastPos;
  base::tick_t m_startTick = 0;
  Event::MouseButton m_button = Event::LeftButton;
  bool m_longPressFired = false;
  int m_maxFingers = 0;
  bool m_gestureMoved = false;
  bool m_gestureDirty = false;
  gfx::PointF m_gestureStartCentroid;
  gfx::PointF m_gestureLastCentroid;
  float m_gestureLastSpread = 0.0f;
  gfx::PointF m_panRemainder;
};

} // namespace os

#endif
