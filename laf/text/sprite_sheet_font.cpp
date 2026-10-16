// LAF Text Library
// Copyright (c) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/sprite_sheet_font.h"

#include "base/utf8_decode.h"
#include "os/sampling.h"
#include "text/font_metrics.h"
#include "text/sprite_sheet_typeface.h"

#include <algorithm>
#include <cmath>

namespace text {

SpriteSheetFont::SpriteSheetFont(const base::Ref<SpriteSheetTypeface>& typeface, float size)
  : m_typeface(typeface)
{
  setSize(size);
}

TypefaceRef SpriteSheetFont::typeface() const
{
  return m_typeface;
}

float SpriteSheetFont::metrics(FontMetrics* metrics) const
{
  // TODO impl

  const float defaultSize = this->defaultSize();

  if (metrics) {
    float descent = m_descent;
    if (m_descent > 0.0f && defaultSize > 0.0f && defaultSize != m_size)
      descent = m_descent * m_size / defaultSize;

    metrics->descent = descent;
    metrics->ascent = -m_size + descent;
    metrics->underlineThickness = 1.0f;
    metrics->underlinePosition = m_descent;
  }

  return lineHeight();
}

float SpriteSheetFont::defaultSize() const
{
  return m_typeface->defaultSize();
}

float SpriteSheetFont::textLength(const std::string& str) const
{
  base::utf8_decode decode(str);
  int x = 0;
  while (int chr = decode.next())
    x += getCharBounds(chr).w;
  return x;
}

float SpriteSheetFont::measureText(const std::string& str,
                                   gfx::RectF* bounds,
                                   const os::Paint* paint) const
{
  float w = textLength(str);
  if (bounds)
    *bounds = gfx::RectF(0, 0, w, lineHeight());
  return w;
}

void SpriteSheetFont::setSize(const float size)
{
  const float defaultSize = this->defaultSize();
  ASSERT(defaultSize > 0.0f);

  // Limit the size of the sprite sheet font to multiples of its own
  // size (x1, x2, x3, etc.)
  int scale = std::max<int>(1, std::floor(size / defaultSize));

  // Limit the scale to the well known maximum size (by memory restrictions).
  const int maxScale = m_typeface->maxScale();
  if (maxScale > 0)
    scale = std::min(scale, maxScale);

  do {
    try {
      // TODO We should be able to use the original sheet scaled in
      // the rendering process (instead of scaling the surface). Right
      // now this simplifies our code but consumes a lot of memory for
      // big font sizes.
      m_sheet = m_typeface->sheetSurface()->applyScale(scale, os::Sampling{});
      break;
    }
    // If an exception is thrown it means that there is not enough
    // memory to scale the font, we have to reduce the scale and try
    // again.
    catch (...) {
      if (scale == 1)
        throw;
      scale /= 2;

      // Mark this new scale as new possible max size.
      m_typeface->setMaxScale(scale);
    }
  } while (scale > 1);
  m_size = scale * defaultSize;
  m_glyphs = m_typeface->glyphs();
  for (auto& rc : m_glyphs)
    rc = gfx::Rect(gfx::RectF(rc) * scale);
}

} // namespace text
