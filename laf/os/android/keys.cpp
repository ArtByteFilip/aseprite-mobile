// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/android/keys.h"

#include <android/input.h>
#include <android/keycodes.h>

namespace os {

KeyScancode android_keycode_to_scancode(int keyCode)
{
  if (keyCode >= AKEYCODE_A && keyCode <= AKEYCODE_Z)
    return KeyScancode(kKeyA + (keyCode - AKEYCODE_A));
  if (keyCode >= AKEYCODE_0 && keyCode <= AKEYCODE_9)
    return KeyScancode(kKey0 + (keyCode - AKEYCODE_0));
  if (keyCode >= AKEYCODE_NUMPAD_0 && keyCode <= AKEYCODE_NUMPAD_9)
    return KeyScancode(kKey0Pad + (keyCode - AKEYCODE_NUMPAD_0));
  if (keyCode >= AKEYCODE_F1 && keyCode <= AKEYCODE_F12)
    return KeyScancode(kKeyF1 + (keyCode - AKEYCODE_F1));

  switch (keyCode) {
    case AKEYCODE_BACK:
    case AKEYCODE_ESCAPE:          return kKeyEsc;
    case AKEYCODE_GRAVE:           return kKeyTilde;
    case AKEYCODE_MINUS:           return kKeyMinus;
    case AKEYCODE_EQUALS:          return kKeyEquals;
    case AKEYCODE_DEL:             return kKeyBackspace;
    case AKEYCODE_TAB:             return kKeyTab;
    case AKEYCODE_LEFT_BRACKET:    return kKeyOpenbrace;
    case AKEYCODE_RIGHT_BRACKET:   return kKeyClosebrace;
    case AKEYCODE_ENTER:           return kKeyEnter;
    case AKEYCODE_SEMICOLON:       return kKeyColon;
    case AKEYCODE_APOSTROPHE:      return kKeyQuote;
    case AKEYCODE_BACKSLASH:       return kKeyBackslash;
    case AKEYCODE_COMMA:           return kKeyComma;
    case AKEYCODE_PERIOD:          return kKeyStop;
    case AKEYCODE_SLASH:           return kKeySlash;
    case AKEYCODE_SPACE:           return kKeySpace;
    case AKEYCODE_INSERT:          return kKeyInsert;
    case AKEYCODE_FORWARD_DEL:     return kKeyDel;
    case AKEYCODE_MOVE_HOME:       return kKeyHome;
    case AKEYCODE_MOVE_END:        return kKeyEnd;
    case AKEYCODE_PAGE_UP:         return kKeyPageUp;
    case AKEYCODE_PAGE_DOWN:       return kKeyPageDown;
    case AKEYCODE_DPAD_LEFT:       return kKeyLeft;
    case AKEYCODE_DPAD_RIGHT:      return kKeyRight;
    case AKEYCODE_DPAD_UP:         return kKeyUp;
    case AKEYCODE_DPAD_DOWN:       return kKeyDown;
    case AKEYCODE_NUMPAD_DIVIDE:   return kKeySlashPad;
    case AKEYCODE_NUMPAD_MULTIPLY: return kKeyAsterisk;
    case AKEYCODE_NUMPAD_SUBTRACT: return kKeyMinusPad;
    case AKEYCODE_NUMPAD_ADD:      return kKeyPlusPad;
    case AKEYCODE_NUMPAD_DOT:      return kKeyDelPad;
    case AKEYCODE_NUMPAD_ENTER:    return kKeyEnterPad;
    case AKEYCODE_NUMPAD_EQUALS:   return kKeyEqualsPad;
    case AKEYCODE_SYSRQ:           return kKeyPrtscr;
    case AKEYCODE_BREAK:           return kKeyPause;
    case AKEYCODE_YEN:             return kKeyYen;
    case AKEYCODE_KANA:            return kKeyKana;
    case AKEYCODE_HENKAN:          return kKeyConvert;
    case AKEYCODE_MUHENKAN:        return kKeyNoconvert;
    case AKEYCODE_AT:              return kKeyAt;
    case AKEYCODE_SHIFT_LEFT:      return kKeyLShift;
    case AKEYCODE_SHIFT_RIGHT:     return kKeyRShift;
    case AKEYCODE_CTRL_LEFT:       return kKeyLControl;
    case AKEYCODE_CTRL_RIGHT:      return kKeyRControl;
    case AKEYCODE_ALT_LEFT:        return kKeyAlt;
    case AKEYCODE_ALT_RIGHT:       return kKeyAltGr;
    case AKEYCODE_META_LEFT:       return kKeyLWin;
    case AKEYCODE_META_RIGHT:      return kKeyRWin;
    case AKEYCODE_MENU:            return kKeyMenu;
    case AKEYCODE_SCROLL_LOCK:     return kKeyScrLock;
    case AKEYCODE_NUM_LOCK:        return kKeyNumLock;
    case AKEYCODE_CAPS_LOCK:       return kKeyCapsLock;
  }
  return kKeyNil;
}

KeyModifiers android_meta_state_to_modifiers(int metaState)
{
  int mods = kKeyNoneModifier;
  if (metaState & AMETA_SHIFT_ON)
    mods |= kKeyShiftModifier;
  if (metaState & AMETA_CTRL_ON)
    mods |= kKeyCtrlModifier;
  if (metaState & AMETA_ALT_ON)
    mods |= kKeyAltModifier;
  if (metaState & AMETA_META_ON)
    mods |= kKeyWinModifier;
  return KeyModifiers(mods);
}

} // namespace os
