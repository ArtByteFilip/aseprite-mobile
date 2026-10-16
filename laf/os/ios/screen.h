// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_SCREEN_H
#define OS_IOS_SCREEN_H
#pragma once

#include "os/ios/ios_app.h"
#include "os/screen.h"
#include "os/system.h"

namespace os {

class ScreenIOS : public Screen {
public:
  ScreenIOS() : m_bounds(gfx::Rect(ios_view_pixel_size()))
  {
    if (m_bounds.isEmpty())
      m_bounds = gfx::Rect(0, 0, 2048, 1536);
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
