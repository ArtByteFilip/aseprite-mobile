// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_KEYS_INCLUDED
#define OS_IOS_KEYS_INCLUDED
#pragma once

#include "os/keys.h"

namespace os {

// Converts a USB HID usage code (UIKey.keyCode) into a KeyScancode
KeyScancode ios_hid_usage_to_scancode(long usage);

} // namespace os

#endif
