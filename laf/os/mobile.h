// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_MOBILE_H_INCLUDED
#define OS_MOBILE_H_INCLUDED
#pragma once

// Functions only available on mobile platforms (Android and iOS)
#if LAF_MOBILE

  #include "os/keys.h"

  #include <functional>
  #include <string>

namespace os {

// Touch gestures recognized by the platform that the app can map to
// commands (e.g. undo/redo).
enum class TouchGesture {
  TwoFingerTap,
  ThreeFingerTap,
};

using TouchGestureHandler = std::function<void(TouchGesture)>;
void set_touch_gesture_handler(TouchGestureHandler&& handler);

// Function called (from the UI thread) when the app goes to
// background. Useful to save backups as the OS can kill the process
// at any moment after this.
using AppPauseHandler = std::function<void()>;
void set_app_pause_handler(AppPauseHandler&& handler);

// When the right-click mode is enabled, the next touch on the screen
// is converted to a right mouse button click/drag (it's disabled
// automatically after that touch).
void set_right_click_mode(bool state);
bool right_click_mode();

// Virtual modifiers (Shift/Ctrl/Alt/Cmd/Space) activated from
// on-screen buttons. These modifiers are added to all mouse/keyboard
// events and reported by os::System::isKeyPressed().
void set_virtual_modifiers(KeyModifiers modifiers);
KeyModifiers virtual_modifiers();

// Shows/hides the on-screen keyboard.
void show_soft_keyboard(bool state);

// Recommended window scale for the current screen density and size
// (used in the first run).
int default_screen_scale();

// Text clipboard of the OS.
bool set_clipboard_text(const std::string& text);
bool get_clipboard_text(std::string& text);

// Internal functions used by the platform backends
void call_touch_gesture_handler(TouchGesture gesture);
void call_app_pause_handler();
bool consume_right_click_mode();

} // namespace os

#endif // LAF_MOBILE

#endif
