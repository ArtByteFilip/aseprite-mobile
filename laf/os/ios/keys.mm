// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/ios/keys.h"

namespace os {

// USB HID usage codes (keyboard page)
KeyScancode ios_hid_usage_to_scancode(long usage)
{
  if (usage >= 0x04 && usage <= 0x1D) // a-z
    return KeyScancode(kKeyA + (usage - 0x04));
  if (usage >= 0x1E && usage <= 0x26) // 1-9
    return KeyScancode(kKey1 + (usage - 0x1E));
  if (usage >= 0x3A && usage <= 0x45) // F1-F12
    return KeyScancode(kKeyF1 + (usage - 0x3A));
  if (usage >= 0x59 && usage <= 0x61) // Keypad 1-9
    return KeyScancode(kKey1Pad + (usage - 0x59));

  switch (usage) {
    case 0x27: return kKey0;
    case 0x28: return kKeyEnter;
    case 0x29: return kKeyEsc;
    case 0x2A: return kKeyBackspace;
    case 0x2B: return kKeyTab;
    case 0x2C: return kKeySpace;
    case 0x2D: return kKeyMinus;
    case 0x2E: return kKeyEquals;
    case 0x2F: return kKeyOpenbrace;
    case 0x30: return kKeyClosebrace;
    case 0x31: return kKeyBackslash;
    case 0x32: return kKeyBackslash2;
    case 0x33: return kKeyColon;
    case 0x34: return kKeyQuote;
    case 0x35: return kKeyTilde;
    case 0x36: return kKeyComma;
    case 0x37: return kKeyStop;
    case 0x38: return kKeySlash;
    case 0x39: return kKeyCapsLock;
    case 0x46: return kKeyPrtscr;
    case 0x47: return kKeyScrLock;
    case 0x48: return kKeyPause;
    case 0x49: return kKeyInsert;
    case 0x4A: return kKeyHome;
    case 0x4B: return kKeyPageUp;
    case 0x4C: return kKeyDel;
    case 0x4D: return kKeyEnd;
    case 0x4E: return kKeyPageDown;
    case 0x4F: return kKeyRight;
    case 0x50: return kKeyLeft;
    case 0x51: return kKeyDown;
    case 0x52: return kKeyUp;
    case 0x53: return kKeyNumLock;
    case 0x54: return kKeySlashPad;
    case 0x55: return kKeyAsterisk;
    case 0x56: return kKeyMinusPad;
    case 0x57: return kKeyPlusPad;
    case 0x58: return kKeyEnterPad;
    case 0x62: return kKey0Pad;
    case 0x63: return kKeyDelPad;
    case 0x67: return kKeyEqualsPad;
    case 0xE0: return kKeyLControl;
    case 0xE1: return kKeyLShift;
    case 0xE2: return kKeyAlt;
    case 0xE3: return kKeyCommand;
    case 0xE4: return kKeyRControl;
    case 0xE5: return kKeyRShift;
    case 0xE6: return kKeyAltGr;
    case 0xE7: return kKeyCommand;
  }
  return kKeyNil;
}

} // namespace os
