// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/ios/system.h"

#include "os/ios/window.h"
#include "os/mobile.h"

#include <array>
#include <atomic>

namespace os {

namespace {

std::array<std::atomic<bool>, kKeyScancodes> g_keys;

} // anonymous namespace

SystemIOS::~SystemIOS()
{
  destroyInstance();
}

// static
void SystemIOS::setKeyPressed(KeyScancode scancode, bool state)
{
  if (scancode > kKeyNil && scancode < kKeyScancodes)
    g_keys[scancode] = state;
}

// static
void SystemIOS::resetAllKeys()
{
  for (auto& key : g_keys)
    key = false;
}

bool SystemIOS::isKeyPressed(KeyScancode scancode)
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

int SystemIOS::getUnicodeFromScancode(KeyScancode scancode)
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
    default:             break;
  }
  return 0;
}

gfx::Point SystemIOS::mousePosition() const
{
  if (WindowIOS* window = WindowIOS::instance()) {
    const gfx::Point pos = window->lastMousePos();
    return gfx::Point(pos.x * window->scale(), pos.y * window->scale());
  }
  return gfx::Point(0, 0);
}

} // namespace os
