// LAF Text Library
// Copyright (c) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "text/font_mgr.h"

#include "gfx/rect.h"
#include "text/font.h"
#include "text/font_style.h"
#include "text/font_style_set.h"
#include "text/typeface.h"

namespace text {

class EmptyTypeface : public Typeface {
public:
  EmptyTypeface() {}
  std::string familyName() const override { return std::string(); }
  FontStyle fontStyle() const override { return FontStyle(); }
};

class EmptyFont : public Font {
public:
  EmptyFont() {}
  FontType type() override { return FontType::Unknown; }
  TypefaceRef typeface() const override { return base::make_ref<EmptyTypeface>(); }
  float metrics(FontMetrics*) const { return 0.0f; }
  float size() const override { return 0.0f; }
  float lineHeight() const override { return 0.0f; }
  float textLength(const std::string&) const override { return 0.0f; };
  float measureText(const std::string&, gfx::RectF*, const os::Paint*) const override
  {
    return 0.0f;
  }
  bool isScalable() const override { return false; }
  void setSize(float) override {}
  bool antialias() const override { return false; }
  void setAntialias(bool) override {}
  FontHinting hinting() const override { return FontHinting::None; }
  void setHinting(FontHinting) override {}

  glyph_t codePointToGlyph(codepoint_t) const override { return false; }
  gfx::RectF getGlyphBounds(glyph_t) const override { return gfx::RectF(); }
  float getGlyphAdvance(glyph_t glyph) const override { return 0; }
};

class EmptyFontStyleSet : public FontStyleSet {
public:
  EmptyFontStyleSet() {}
  int count() override { return 0; }
  void getStyle(int index, FontStyle& style, std::string& name) override {}
  TypefaceRef typeface(int index) override { return base::make_ref<EmptyTypeface>(); }
  TypefaceRef matchStyle(const FontStyle& style) override
  {
    return base::make_ref<EmptyTypeface>();
  }
};

class EmptyFontMgr : public FontMgr {
public:
  EmptyFontMgr() {}
  ~EmptyFontMgr() {}

  FontRef makeFont(const TypefaceRef& typeface) { return base::make_ref<EmptyFont>(); }
  FontRef makeFont(const TypefaceRef& typeface, float size) { return base::make_ref<EmptyFont>(); }

  FontRef defaultFont(float size) const override { return base::make_ref<EmptyFont>(); }
  int countFamilies() const override { return 0; }
  std::string familyName(int i) const override { return std::string(); }
  FontStyleSetRef familyStyleSet(int i) const override
  {
    return base::make_ref<EmptyFontStyleSet>();
  }
  FontStyleSetRef matchFamily(const std::string& familyName) const override
  {
    return base::make_ref<EmptyFontStyleSet>();
  }
};

// static
FontMgrRef FontMgr::Make()
{
  return base::make_ref<EmptyFontMgr>();
}

} // namespace text
