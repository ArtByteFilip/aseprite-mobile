// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_ANDROID_KEYS_INCLUDED
#define OS_ANDROID_KEYS_INCLUDED
#pragma once

#include "os/keys.h"

namespace os {

KeyScancode android_keycode_to_scancode(int keyCode);
KeyModifiers android_meta_state_to_modifiers(int metaState);

} // namespace os

#endif
