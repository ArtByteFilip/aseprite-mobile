// LAF Text Library
// Copyright (C) 2019-2025  Igara Studio S.A.
// Copyright (C) 2012-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_SPRITE_SHEET_FONT_H_INCLUDED
#define LAF_TEXT_SPRITE_SHEET_FONT_H_INCLUDED
#pragma once

#include "base/ref.h"
#include "gfx/rect.h"
#include "os/surface.h"
#include "text/font.h"

#include <vector>

namespace text {

class SpriteSheetTypeface;

class SpriteSheetFont : public Font {
public:
  SpriteSheetFont(const base::Ref<SpriteSheetTypeface>& typeface, float size);

  FontType type() override { return FontType::SpriteSheet; }

  TypefaceRef typeface() const override;

  void setDescent(float descent) { m_descent = descent; }

  float metrics(FontMetrics* metrics) const override;

  float defaultSize() const override;
  float size() const override { return m_size; }
  float lineHeight() const override { return m_size; }
  float textLength(const std::string& str) const override;
  float measureText(const std::string& str,
                    gfx::RectF* bounds,
                    const os::Paint* paint) const override;

  bool isScalable() const override { return true; }

  void setSize(float size) override;

  bool antialias() const override { return m_antialias; }

  void setAntialias(bool antialias) override
  {
    m_antialias = antialias;
    setSize(m_size);
  }

  FontHinting hinting() const override { return FontHinting::None; }

  void setHinting(FontHinting hinting) override { (void)hinting; }

  glyph_t codePointToGlyph(const codepoint_t codepoint) const override
  {
    glyph_t glyph = codepoint - int(' ') + 2;
    if (glyph >= 0 && glyph < int(m_glyphs.size()) && !m_glyphs[glyph].isEmpty()) {
      return glyph;
    }
    else
      return 0;
  }

  gfx::RectF getGlyphBounds(glyph_t glyph) const override
  {
    if (glyph >= 0 && glyph < (int)m_glyphs.size())
      return gfx::RectF(0, 0, m_glyphs[glyph].w, m_glyphs[glyph].h);

    return getCharBounds(128);
  }

  float getGlyphAdvance(glyph_t glyph) const override { return getGlyphBounds(glyph).w; }

  gfx::RectF getGlyphBoundsOnSheet(glyph_t glyph) const
  {
    if (glyph >= 0 && glyph < (int)m_glyphs.size())
      return m_glyphs[glyph];

    return getCharBounds(128);
  }

  gfx::RectF getGlyphBoundsOutput(glyph_t glyph) const
  {
    gfx::RectF bounds = getGlyphBoundsOnSheet(glyph);
    return bounds * m_size;
  }

  os::Surface* sheetSurface() const { return m_sheet.get(); }

  gfx::Rect getCharBounds(codepoint_t cp) const
  {
    glyph_t glyph = codePointToGlyph(cp);
    if (glyph == 0)
      glyph = codePointToGlyph(128);

    if (glyph != 0)
      return m_glyphs[glyph];
    else
      return gfx::Rect();
  }

private:
  base::Ref<SpriteSheetTypeface> m_typeface;
  os::SurfaceRef m_sheet;
  std::vector<gfx::Rect> m_glyphs;
  float m_size = 0.0f;
  float m_descent = 0.0f;
  bool m_antialias = false;
};

} // namespace text

#endif
