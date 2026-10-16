// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_ANDROID_SCREEN_H
#define OS_ANDROID_SCREEN_H
#pragma once

#include "os/android/window.h"
#include "os/screen.h"
#include "os/system.h"

namespace os {

class ScreenAndroid : public Screen {
public:
  ScreenAndroid()
  {
    if (WindowAndroid* window = WindowAndroid::instance())
      m_bounds = gfx::Rect(window->clientSize());
    else
      m_bounds = gfx::Rect(0, 0, 1280, 720);
  }

  bool isPrimary() const override { return true; }
  gfx::Rect bounds() const override { return m_bounds; }
  gfx::Rect workarea() const override { return m_bounds; }

  os::ColorSpaceRef colorSpace() const override
  {
    return System::instance()->makeColorSpace(gfx::ColorSpace::MakeSRGB());
  }

  void* nativeHandle() const override { return nullptr; }

private:
  gfx::Rect m_bounds;
};

} // namespace os

#endif
