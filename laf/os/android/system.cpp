// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/android/system.h"

#include "os/android/android_app.h"
#include "os/android/window.h"

#include <array>
#include <atomic>

namespace os {

namespace {

std::array<std::atomic<bool>, kKeyScancodes> g_keys;

} // anonymous namespace

SystemAndroid::~SystemAndroid()
{
  destroyInstance();
}

// static
void SystemAndroid::setKeyPressed(KeyScancode scancode, bool state)
{
  if (scancode > kKeyNil && scancode < kKeyScancodes)
    g_keys[scancode] = state;
}

// static
void SystemAndroid::resetAllKeys()
{
  for (auto& key : g_keys)
    key = false;
}

bool SystemAndroid::isKeyPressed(KeyScancode scancode)
{
  if (scancode <= kKeyNil || scancode >= kKeyScancodes)
    return false;

  // Virtual modifiers from on-screen buttons
  const KeyModifiers mods = virtual_modifiers();
  switch (scancode) {
    case kKeyLShift:
      if (mods & kKeyShiftModifier)
        return true;
      break;
    case kKeyLControl:
      if (mods & kKeyCtrlModifier)
        return true;
      break;
    case kKeyAlt:
      if (mods & kKeyAltModifier)
        return true;
      break;
    case kKeyCommand:
      if (mods & kKeyCmdModifier)
        return true;
      break;
    case kKeySpace:
      if (mods & kKeySpaceModifier)
        return true;
      break;
    default: break;
  }
  return g_keys[scancode];
}

int SystemAndroid::getUnicodeFromScancode(KeyScancode scancode)
{
  if (scancode >= kKeyA && scancode <= kKeyZ)
    return 'a' + (scancode - kKeyA);
  if (scancode >= kKey0 && scancode <= kKey9)
    return '0' + (scancode - kKey0);
  if (scancode >= kKey0Pad && scancode <= kKey9Pad)
    return '0' + (scancode - kKey0Pad);
  switch (scancode) {
    case kKeyTilde:      return '`';
    case kKeyMinus:      return '-';
    case kKeyEquals:     return '=';
    case kKeyOpenbrace:  return '[';
    case kKeyClosebrace: return ']';
    case kKeyColon:      return ';';
    case kKeyQuote:      return '\'';
    case kKeyBackslash:  return '\\';
    case kKeyComma:      return ',';
    case kKeyStop:       return '.';
    case kKeySlash:      return '/';
    case kKeySpace:      return ' ';
    case kKeySlashPad:   return '/';
    case kKeyAsterisk:   return '*';
    case kKeyMinusPad:   return '-';
    case kKeyPlusPad:    return '+';
    default:             break;
  }
  return 0;
}

gfx::Point SystemAndroid::mousePosition() const
{
  if (WindowAndroid* window = WindowAndroid::instance()) {
    const gfx::Point pos = window->lastMousePos();
    return gfx::Point(pos.x * window->scale(), pos.y * window->scale());
  }
  return gfx::Point(0, 0);
}

} // namespace os
