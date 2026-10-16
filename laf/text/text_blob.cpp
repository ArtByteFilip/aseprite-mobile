// LAF Text Library
// Copyright (c) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/text_blob.h"

#include "gfx/rect.h"
#include "gfx/size.h"
#include "text/font.h"
#include "text/font_metrics.h"
#include "text/sprite_text_blob.h"

#if LAF_SKIA
  #include "text/skia_text_blob.h"
#endif

namespace text {

gfx::RectF TextBlob::bounds()
{
  if (m_bounds.isEmpty()) {
    m_bounds = gfx::RectF(0, 0, 1, 1);
    visitRuns([this](RunInfo& info) {
      for (int i = 0; i < info.glyphCount; ++i) {
        m_bounds |= info.getGlyphBounds(i);
      }
    });
  }
  return m_bounds;
}

float TextBlob::baseline()
{
  if (m_baseline == 0.0f) {
    visitRuns([this](RunInfo& info) {
      if (!info.font)
        return;

      FontMetrics metrics;
      info.font->metrics(&metrics);
      m_baseline = std::max(m_baseline, -metrics.ascent);
    });
  }
  return m_baseline;
}

float TextBlob::textHeight()
{
  if (m_textHeight == 0.0f) {
    visitRuns([this](RunInfo& info) {
      if (!info.font)
        return;

      FontMetrics metrics;
      info.font->metrics(&metrics);
      m_textHeight = std::max(m_textHeight, metrics.descent - metrics.ascent);
    });
  }
  return m_textHeight;
}

TextBlob::Utf8Range TextBlob::RunInfo::getGlyphUtf8Range(size_t i) const
{
  Utf8Range subRange;

  ASSERT(clusters);
  ASSERT(i < glyphCount);
  if (i >= glyphCount || !clusters)
    return subRange;

  // LTR
  if (!rtl) {
    subRange.begin = utf8Range.begin + clusters[i];
    subRange.end = (i + 1 < glyphCount ? utf8Range.begin + clusters[i + 1] : utf8Range.end);
  }
  // RTL
  else {
    subRange.begin = utf8Range.begin + clusters[i];
    subRange.end = (i == 0 ? utf8Range.end : utf8Range.begin + clusters[i - 1]);
  }
  return subRange;
}

gfx::RectF TextBlob::RunInfo::getGlyphBounds(const size_t i) const
{
  ASSERT(i < glyphCount);
  if (i >= glyphCount)
    return gfx::RectF();

  gfx::RectF bounds = font->getGlyphBounds(glyphs[i]);

  // Get bounds of whitespace from a space glyph.
  if (bounds.isEmpty()) {
    FontMetrics metrics;
    font->metrics(&metrics);
    bounds.w = font->getGlyphAdvance(font->codePointToGlyph(' '));
    bounds.h = 1;
  }

  if (bounds.isEmpty())
    return bounds;

  bounds.offset(positions[i]);
  if (offsets)
    bounds.offset(offsets[i]);

  // Add global "point" offset to the bounds.
  bounds.offset(point);
  return bounds;
}

TextBlobRef TextBlob::Make(const FontRef& font, const std::string& text)
{
  ASSERT(font);
  switch (font->type()) {
    case FontType::SpriteSheet: return SpriteTextBlob::Make(font, text);

    case FontType::FreeType:
      ASSERT(false); // TODO impl
      return nullptr;

#if LAF_SKIA
    case FontType::Native: return SkiaTextBlob::Make(font, text);
#endif

    default: return nullptr;
  }
}

} // namespace text
