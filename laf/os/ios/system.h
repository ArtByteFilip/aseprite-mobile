// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_SYSTEM_H
#define OS_IOS_SYSTEM_H
#pragma once

#include "os/common/system.h"
#include "os/ios/screen.h"

namespace os {

class SystemIOS : public CommonSystem {
public:
  ~SystemIOS();

  void setTabletOptions(const TabletOptions& options) override { m_tabletOptions = options; }
  TabletOptions tabletOptions() const override { return m_tabletOptions; }

  bool isKeyPressed(KeyScancode scancode) override;
  void resetKeyPressed() override {}
  int getUnicodeFromScancode(KeyScancode scancode) override;

  CursorRef makeCursor(const Surface* surface, const gfx::Point& focus, const int scale) override
  {
    return nullptr;
  }

  gfx::Point mousePosition() const override;
  void setMousePosition(const gfx::Point& screenPosition) override {}

  gfx::Color getColorFromScreen(const gfx::Point& screenPosition) const override
  {
    return gfx::ColorNone;
  }

  ScreenRef primaryScreen() override { return make_ref<ScreenIOS>(); }
  void listScreens(ScreenList& list) override { list.push_back(make_ref<ScreenIOS>()); }

  static void setKeyPressed(KeyScancode scancode, bool state);
  static void resetAllKeys();

private:
  TabletOptions m_tabletOptions;
};

} // namespace os

#endif
