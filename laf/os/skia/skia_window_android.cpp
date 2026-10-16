// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/skia/skia_window_android.h"

#include "os/skia/skia_surface.h"
#include "os/skia/skia_window.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"

#include <android/native_window.h>

namespace os {

SkiaWindowAndroid::SkiaWindowAndroid(const WindowSpec& spec) : Base(spec)
{
  initColorSpace();
}

void SkiaWindowAndroid::onPaint(ANativeWindow* nativeWindow, const gfx::Region& rgn)
{
  auto surface = static_cast<SkiaSurface*>(this->surface());
  if (!surface || !nativeWindow)
    return;

  const gfx::Rect bounds = rgn.bounds() & gfx::Rect(clientSize());
  if (bounds.isEmpty())
    return;

  ARect dirty;
  dirty.left = bounds.x;
  dirty.top = bounds.y;
  dirty.right = bounds.x2();
  dirty.bottom = bounds.y2();

  ANativeWindow_Buffer buffer;
  if (ANativeWindow_lock(nativeWindow, &buffer, &dirty) != 0)
    return;

  // The dirty area could be bigger than the requested one (e.g. the
  // whole window if the buffer content is not preserved).
  const SkIRect dstRect = SkIRect::MakeLTRB(dirty.left, dirty.top, dirty.right, dirty.bottom);

  SkBitmap dst;
  const SkImageInfo info =
    SkImageInfo::Make(buffer.width, buffer.height, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
  if (dst.installPixels(info, buffer.bits, size_t(buffer.stride) * 4)) {
    const SkBitmap& src = surface->bitmap();
    const int scale = this->scale();

    SkCanvas canvas(dst);
    canvas.clipIRect(dstRect);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    // Pixels outside the surface (when the window size is not a
    // multiple of the scale) are painted with black.
    canvas.clear(SK_ColorBLACK);

    canvas.scale(SkScalar(scale), SkScalar(scale));
    canvas.drawImage(SkImages::RasterFromPixmap(src.pixmap(), nullptr, nullptr),
                     0,
                     0,
                     SkSamplingOptions(SkFilterMode::kNearest),
                     &paint);
  }

  ANativeWindow_unlockAndPost(nativeWindow);
}

} // namespace os
