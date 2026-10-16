// LAF Text Library
// Copyright (C) 2020-2025  Igara Studio S.A.
// Copyright (C) 2016-2017  David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FREETYPE_FONT_H_INCLUDED
#define LAF_TEXT_FREETYPE_FONT_H_INCLUDED
#pragma once

#include "ft/hb_face.h"
#include "ft/lib.h"
#include "text/font.h"

namespace text {
class Font;

class FreeTypeFont : public Font {
public:
  using Face = ft::Face;

  FreeTypeFont(ft::Lib& lib, const char* filename, const int height);
  ~FreeTypeFont();

  bool isValid() const;
  FontType type() override;
  TypefaceRef typeface() const override;
  float metrics(FontMetrics* metrics) const override;
  float size() const override;
  float lineHeight() const override;
  float textLength(const std::string& str) const override;
  float measureText(const std::string& str,
                    gfx::RectF* bounds,
                    const os::Paint* paint) const override;
  bool isScalable() const override;
  void setSize(float size) override;
  bool antialias() const override;
  void setAntialias(bool antialias) override;
  FontHinting hinting() const override;
  void setHinting(FontHinting hinting) override;

  glyph_t codePointToGlyph(codepoint_t cp) const override;
  gfx::RectF getGlyphBounds(glyph_t glyph) const override;
  float getGlyphAdvance(glyph_t glyph) const override;

  Face& face() { return m_face; }

  static base::Ref<FreeTypeFont> LoadFont(ft::Lib& lib, const char* filename, const int height);

private:
  mutable Face m_face;
  text::FontHinting m_hinting;
};

} // namespace text

#endif
