// LAF Text Library
// Copyright (c) 2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/sprite_sheet_typeface.h"

#include "gfx/color.h"
#include "os/system.h"
#include "text/font_style.h"

namespace text {

static constexpr auto kRedColor = gfx::rgba(255, 0, 0);

std::string SpriteSheetTypeface::familyName() const
{
  return {};
}

FontStyle SpriteSheetTypeface::fontStyle() const
{
  return {};
}

// static
base::Ref<SpriteSheetTypeface> SpriteSheetTypeface::FromFile(const char* filename)
{
  auto typeface = base::make_ref<SpriteSheetTypeface>();
  if (!typeface->fromFile(filename))
    return nullptr;
  return typeface;
}

bool SpriteSheetTypeface::fromFile(const char* filename)
{
  m_sheet = os::System::instance()->loadRgbaSurface(filename);
  if (!m_sheet)
    return false;

  m_glyphs.push_back(gfx::Rect()); // glyph index 0 is MISSING CHARACTER glyph
  m_glyphs.push_back(gfx::Rect()); // glyph index 1 is NULL glyph

  os::Surface* sur = m_sheet.get();
  os::SurfaceLock lock(sur);
  gfx::Rect bounds(0, 0, 1, 1);
  gfx::Rect glyphBounds;

  while (findGlyph(sur, sur->width(), sur->height(), bounds, glyphBounds)) {
    m_glyphs.push_back(glyphBounds);
    bounds.x += bounds.w;
  }

  // Clear the border of all glyphs to avoid bilinear interpolation
  // with those borders when drawing this font scaled/antialised.
  os::Paint p;
  p.blendMode(os::BlendMode::Clear);
  p.style(os::Paint::Stroke);
  for (gfx::Rect rc : m_glyphs) {
    sur->drawRect(rc.enlarge(1), p);
  }

  m_defaultSize = (m_glyphs.size() > 2 ? m_glyphs[2].h : 0.0f);
  if (m_defaultSize <= 0.0f)
    m_defaultSize = 1.0f;

  m_sheet->setImmutable();
  return true;
}

bool SpriteSheetTypeface::findGlyph(const os::Surface* sur,
                                    int width,
                                    int height,
                                    gfx::Rect& bounds,
                                    gfx::Rect& glyphBounds)
{
  gfx::Color keyColor = sur->getPixel(0, 0);

  while (sur->getPixel(bounds.x, bounds.y) == keyColor) {
    bounds.x++;
    if (bounds.x >= width) {
      bounds.x = 0;
      bounds.y += bounds.h;
      bounds.h = 1;
      if (bounds.y >= height)
        return false;
    }
  }

  gfx::Color firstCharPixel = sur->getPixel(bounds.x, bounds.y);

  bounds.w = 0;
  while ((bounds.x + bounds.w < width) &&
         (sur->getPixel(bounds.x + bounds.w, bounds.y) != keyColor)) {
    bounds.w++;
  }

  bounds.h = 0;
  while ((bounds.y + bounds.h < height) &&
         (sur->getPixel(bounds.x, bounds.y + bounds.h) != keyColor)) {
    bounds.h++;
  }

  // Using red color in the first pixel of the char indicates that
  // this glyph shouldn't be used as a valid one.
  if (firstCharPixel != kRedColor)
    glyphBounds = bounds;
  else
    glyphBounds = gfx::Rect();

  return !bounds.isEmpty();
}

} // namespace text
