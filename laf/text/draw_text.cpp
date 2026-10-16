// LAF Text Library
// Copyright (C) 2020-2024  Igara Studio S.A.
// Copyright (C) 2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/draw_text.h"

#include "gfx/clip.h"
#include "os/paint.h"
#include "os/surface.h"
#include "text/sprite_sheet_font.h"
#include "text/sprite_text_blob.h"
#include "text/text_blob.h"

#if LAF_FREETYPE
  #include "ft/algorithm.h"
  #include "ft/hb_shaper.h"
  #include "text/freetype_font.h"
#endif

#if LAF_SKIA
  #include "os/skia/skia_helpers.h"
  #include "os/skia/skia_surface.h"
  #include "text/skia_font.h"
  #include "text/skia_text_blob.h"

  #include "include/core/SkCanvas.h"
#endif

namespace text {

void draw_text(os::Surface* surface,
               const FontRef& font,
               const std::string& text,
               gfx::PointF pos,
               const os::Paint* paint,
               const TextAlign textAlign)
{
  ASSERT(surface);
  if (!surface)
    return;

  const TextBlobRef blob = TextBlob::Make(font, text);
  if (!blob)
    return;

  switch (textAlign) {
    case TextAlign::Left:   break;
    case TextAlign::Center: pos.x -= blob->bounds().w / 2.0f; break;
    case TextAlign::Right:  pos.x -= blob->bounds().w; break;
  }

  draw_text(surface, blob, pos, paint);
}

void draw_text(os::Surface* surface,
               const TextBlobRef& blob,
               const gfx::PointF& pos,
               const os::Paint* paint)
{
  ASSERT(surface);
  ASSERT(blob);
  if (!surface || !blob)
    return;

#if LAF_SKIA
  if (const auto* skiaBlob = dynamic_cast<const SkiaTextBlob*>(blob.get())) {
    static_cast<os::SkiaSurface*>(surface)->canvas().drawTextBlob(
      skiaBlob->skTextBlob(),
      pos.x,
      pos.y,
      (paint ? paint->skPaint() : SkPaint()));
  }
#endif

  if (const auto* spriteBlob = dynamic_cast<const SpriteTextBlob*>(blob.get())) {
    const auto* spriteFont = static_cast<const SpriteSheetFont*>(spriteBlob->font().get());
    const os::Surface* sheet = spriteFont->sheetSurface();

    for (const auto& run : spriteBlob->runs()) {
      if (run.subBlob) {
        gfx::PointF subPos = pos;
        if (!run.positions.empty())
          subPos += run.positions[0];
        draw_text(surface, run.subBlob, subPos, paint);
        continue;
      }

      const size_t n = run.glyphs.size();
      for (int i = 0; i < n; ++i) {
        const gfx::Rect glyphBounds = spriteFont->getGlyphBoundsOnSheet(run.glyphs[i]);

        surface->drawColoredRgbaSurface(sheet,
                                        (paint ? paint->color() : gfx::ColorNone),
                                        gfx::ColorNone,
                                        gfx::Clip(gfx::Point(run.positions[i] + pos), glyphBounds));
      }
    }
  }

  // TODO impl
}

} // namespace text
