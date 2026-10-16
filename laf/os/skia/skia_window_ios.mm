// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/skia/skia_window_ios.h"

#include "os/ios/ios_app.h"
#include "os/ios/view.h"
#include "os/skia/skia_surface.h"
#include "os/skia/skia_window.h"

#include "include/core/SkBitmap.h"

#include <CoreGraphics/CoreGraphics.h>

namespace os {

SkiaWindowIOS::SkiaWindowIOS(const WindowSpec& spec) : Base(spec)
{
  initColorSpace();
}

void SkiaWindowIOS::onPaint()
{
  auto surface = static_cast<SkiaSurface*>(this->surface());
  if (!surface)
    return;

  AsepriteView* view = (__bridge AsepriteView*)ios_main_view();
  if (!view)
    return;

  const SkBitmap& bitmap = surface->bitmap();
  if (bitmap.drawsNothing() || !bitmap.getPixels())
    return;

  CGBitmapInfo info;
  switch (bitmap.colorType()) {
    case kBGRA_8888_SkColorType:
      info = kCGBitmapByteOrder32Little | kCGImageAlphaNoneSkipFirst;
      break;
    case kRGBA_8888_SkColorType:
      info = kCGBitmapByteOrder32Big | kCGImageAlphaNoneSkipLast;
      break;
    default: return;
  }

  // Copy the pixels (the surface will be modified while the layer
  // is displaying the image)
  CFDataRef data = CFDataCreate(nullptr,
                                (const UInt8*)bitmap.getPixels(),
                                CFIndex(bitmap.rowBytes() * bitmap.height()));
  CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
  CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  CGImageRef image = CGImageCreate(bitmap.width(),
                                   bitmap.height(),
                                   8,
                                   32,
                                   bitmap.rowBytes(),
                                   cs,
                                   info,
                                   provider,
                                   nullptr,
                                   false,
                                   kCGRenderingIntentDefault);

  [view presentImage:image imageWidth:bitmap.width() imageHeight:bitmap.height() windowScale:scale()];

  CGImageRelease(image);
  CGColorSpaceRelease(cs);
  CGDataProviderRelease(provider);
  CFRelease(data);
}

} // namespace os
