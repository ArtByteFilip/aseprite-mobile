// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_IOS_APP_H_INCLUDED
#define OS_IOS_IOS_APP_H_INCLUDED
#pragma once

#include "gfx/size.h"
#include "os/mobile.h"

#include <string>
#include <vector>

namespace os {

// Processes UIKit events (touches, keys, lifecycle, etc.) running the
// main run loop. If timeoutSecs < 0 it waits until one event is
// received.
void ios_process_events(double timeoutSecs);

// Wakes up the main run loop (when an event is queued from other
// thread).
void ios_wakeup();

// Returns the AsepriteView (UIView*) where the app is rendered.
void* ios_main_view();

// Size of the main view in real pixels.
gfx::Size ios_view_pixel_size();

// Scale factor between UIKit points and real pixels (e.g. 2 on iPads).
float ios_content_scale();

// Short vibration/haptic feedback
void ios_haptic_feedback();

// Opens the given URL with other app (e.g. Safari)
bool ios_open_url(const std::string& url);

// Files received to be opened (e.g. from the Files app)
void ios_add_pending_file(const std::string& fn);
std::vector<std::string> ios_take_pending_files();

} // namespace os

#endif
