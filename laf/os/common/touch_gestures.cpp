// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/common/touch_gestures.h"

#include <algorithm>
#include <cmath>

namespace os {

namespace {

// Time to recognize a long-press (converted to a right-click)
constexpr base::tick_t kLongPressMsecs = 450;

// Max duration of a two/three fingers tap (undo/redo)
constexpr base::tick_t kGestureTapMsecs = 300;

// Distance (in dp) that a finger must move to start dragging
constexpr float kTouchSlopDp = 6.0f;

// Distance (in dp) that fingers must move to cancel a gesture tap
constexpr float kGestureSlopDp = 12.0f;

float distance(const gfx::PointF& a, const gfx::PointF& b)
{
  return std::hypot(a.x - b.x, a.y - b.y);
}

} // anonymous namespace

TouchGestures::Finger* TouchGestures::findFinger(int id)
{
  for (auto& f : m_fingers) {
    if (f.id == id)
      return &f;
  }
  return nullptr;
}

gfx::PointF TouchGestures::centroid() const
{
  gfx::PointF c(0, 0);
  if (m_fingers.empty())
    return c;
  for (const auto& f : m_fingers) {
    c.x += f.pos.x;
    c.y += f.pos.y;
  }
  c.x /= m_fingers.size();
  c.y /= m_fingers.size();
  return c;
}

float TouchGestures::spread() const
{
  if (m_fingers.size() < 2)
    return 0.0f;
  const gfx::PointF c = centroid();
  float sum = 0.0f;
  for (const auto& f : m_fingers)
    sum += distance(f.pos, c);
  return sum / m_fingers.size();
}

void TouchGestures::mouse(Event::Type type, const gfx::PointF& pos, Event::MouseButton button)
{
  m_delegate->onTouchMouseEvent(type, pos, button);
}

void TouchGestures::fingerDown(int id, const gfx::PointF& pos)
{
  const base::tick_t now = base::current_tick();

  if (m_fingers.empty()) {
    m_fingers.push_back(Finger{ id, pos });
    m_primaryId = id;
    m_startPos = m_lastPos = pos;
    m_startTick = now;
    m_longPressFired = false;
    m_maxFingers = 1;
    m_button = (m_delegate->onTouchConsumeRightClickMode() ? Event::RightButton :
                                                             Event::LeftButton);
    m_state = State::Pending;

    // Move the mouse to the touched position (without buttons) so
    // the UI can update the hot widget.
    mouse(Event::MouseEnter, pos, Event::NoneButton);
    mouse(Event::MouseMove, pos, Event::NoneButton);
    return;
  }

  if (!findFinger(id))
    m_fingers.push_back(Finger{ id, pos });
  m_maxFingers = std::max<int>(m_maxFingers, m_fingers.size());

  if (m_state == State::Pending || m_state == State::Dragging) {
    // Finish the current one-finger drag
    if (m_state == State::Dragging)
      mouse(Event::MouseUp, m_lastPos, m_button);

    m_state = State::Gesture;
    m_gestureMoved = false;
    m_startTick = now;
    m_panRemainder = gfx::PointF(0, 0);
  }
  if (m_state == State::Gesture)
    startGesture();
}

void TouchGestures::startGesture()
{
  m_gestureStartCentroid = m_gestureLastCentroid = centroid();
  m_gestureLastSpread = spread();
  // Put the mouse in the center of the gesture to zoom/scroll the
  // widget below the fingers.
  mouse(Event::MouseMove, m_gestureLastCentroid, Event::NoneButton);
}

void TouchGestures::fingerMove(int id, const gfx::PointF& pos)
{
  Finger* finger = findFinger(id);
  if (!finger)
    return;
  finger->pos = pos;

  switch (m_state) {
    case State::Pending:
    case State::Dragging:
      if (id != m_primaryId)
        break;
      if (m_state == State::Pending) {
        if (distance(pos, m_startPos) < m_delegate->touchDp(kTouchSlopDp))
          break;
        // A long-press followed by a drag is a normal drag
        if (m_longPressFired)
          m_button = Event::LeftButton;
        m_state = State::Dragging;
        mouse(Event::MouseDown, m_startPos, m_button);
      }
      m_lastPos = pos;
      mouse(Event::MouseMove, pos, Event::NoneButton);
      break;

    case State::Gesture: m_gestureDirty = true; break;

    default:             break;
  }
}

void TouchGestures::endOfMoveBatch()
{
  if (m_state == State::Gesture && m_gestureDirty) {
    m_gestureDirty = false;
    queueGestureEvents();
  }
}

void TouchGestures::fingerUp(int id, const gfx::PointF& pos)
{
  const base::tick_t now = base::current_tick();

  if (Finger* finger = findFinger(id))
    finger->pos = pos;

  // Last finger released
  if (m_fingers.size() <= 1) {
    switch (m_state) {
      case State::Pending:
        // A tap (or a long-press converted to right-click)
        if (m_longPressFired)
          m_button = Event::RightButton;
        mouse(Event::MouseDown, m_startPos, m_button);
        mouse(Event::MouseUp, m_startPos, m_button);
        break;
      case State::Dragging: mouse(Event::MouseUp, pos, m_button); break;
      default:              break;
    }
    m_fingers.clear();
    m_state = State::Idle;
    // There is no "hover" with fingers, so we hide the mouse
    // cursor/brush preview when the finger is released.
    mouse(Event::MouseLeave, pos, Event::NoneButton);
    return;
  }

  m_fingers.erase(std::remove_if(m_fingers.begin(),
                                 m_fingers.end(),
                                 [id](const Finger& f) { return f.id == id; }),
                  m_fingers.end());

  if (m_state == State::Gesture) {
    if (m_fingers.size() >= 2) {
      // Continue the gesture with the remaining fingers
      m_gestureLastCentroid = centroid();
      m_gestureLastSpread = spread();
    }
    else {
      // Check two/three fingers tap
      if (!m_gestureMoved && now - m_startTick < kGestureTapMsecs) {
        if (m_maxFingers == 2)
          m_delegate->onTouchGesture(Gesture::TwoFingerTap);
        else if (m_maxFingers == 3)
          m_delegate->onTouchGesture(Gesture::ThreeFingerTap);
      }
      m_state = State::Finished;
    }
  }
}

void TouchGestures::cancel()
{
  if (m_state == State::Dragging)
    mouse(Event::MouseUp, m_lastPos, m_button);
  if (m_state != State::Idle)
    mouse(Event::MouseLeave, m_lastPos, Event::NoneButton);
  m_state = State::Idle;
  m_fingers.clear();
}

void TouchGestures::queueGestureEvents()
{
  if (m_fingers.size() < 2)
    return;

  const gfx::PointF c = centroid();
  const float s = spread();
  const float slop = m_delegate->touchDp(kGestureSlopDp);

  if (!m_gestureMoved) {
    if (distance(c, m_gestureStartCentroid) > slop || std::fabs(s - m_gestureLastSpread) > slop) {
      m_gestureMoved = true;
      // Reset the reference values to avoid a jump
      m_gestureLastCentroid = c;
      m_gestureLastSpread = s;
    }
    return;
  }

  // Pinch to zoom
  if (m_gestureLastSpread > 0.0f && s > 0.0f) {
    const float magnification = s / m_gestureLastSpread - 1.0f;
    if (std::fabs(magnification) > 0.01f) {
      m_delegate->onTouchMagnify(c, magnification);
      m_gestureLastSpread = s;
    }
  }

  // Pan (scroll) with two fingers
  const int scale = std::max(1, m_delegate->touchScale());
  const gfx::PointF delta(c.x - m_gestureLastCentroid.x + m_panRemainder.x,
                          c.y - m_gestureLastCentroid.y + m_panRemainder.y);
  const int dx = int(delta.x / scale);
  const int dy = int(delta.y / scale);
  m_panRemainder = gfx::PointF(delta.x - dx * scale, delta.y - dy * scale);
  m_gestureLastCentroid = c;

  if (dx != 0 || dy != 0)
    m_delegate->onTouchScroll(c, gfx::Point(-dx, -dy));
}

int TouchGestures::msecsToNextTimer() const
{
  if (m_state == State::Pending && !m_longPressFired) {
    const base::tick_t elapsed = base::current_tick() - m_startTick;
    return int(std::max<base::tick_t>(0, kLongPressMsecs - elapsed));
  }
  return -1;
}

void TouchGestures::processTimers()
{
  if (m_state == State::Pending && !m_longPressFired &&
      base::current_tick() - m_startTick >= kLongPressMsecs) {
    // Long-press recognized: when the finger is released without
    // moving, we'll generate a right-click.
    m_longPressFired = true;
    m_delegate->onTouchLongPress();
  }
}

} // namespace os
