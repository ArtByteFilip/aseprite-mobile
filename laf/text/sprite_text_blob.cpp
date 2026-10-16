// LAF Text Library
// Copyright (c) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/sprite_text_blob.h"

#include "base/ref.h"
#include "base/utf8_decode.h"
#include "text/font.h"
#include "text/font_mgr.h"
#include "text/sprite_sheet_font.h"

namespace text {

namespace {

// Used for sub-TextBlobs created from SpriteTextBlob to handle runs
// of text with callback fonts.
//
// As SpriteTextBlob can create other TextBlobs with sub-strings
// (using other kind of fonts like SkiaFont), those TextBlob
// (SkiaTextBlob) only see the sub-string part for the run (they don't
// know the whole string). With this OffsetHandler we adapt that
// sub-string information to the global string, adjusting the UTF-8
// range and the output position.
class OffsetHandler : public TextBlob::RunHandler {
public:
  OffsetHandler(RunHandler* original, const int offsetUtf8, const gfx::PointF& offsetOrigin)
    : m_original(original)
    , m_offsetUtf8(offsetUtf8)
    , m_offsetOrigin(offsetOrigin)
  {
  }

  // TextBlob::RunHandler impl
  void commitRunBuffer(TextBlob::RunInfo& info) override
  {
    // Adjust UTF8 range and position.
    info.utf8Range.begin += m_offsetUtf8;
    info.utf8Range.end += m_offsetUtf8;
    info.point = m_offsetOrigin;

    // Call the original RunHandler with the global info.
    if (m_original)
      m_original->commitRunBuffer(info);
  }

private:
  RunHandler* m_original;
  int m_offsetUtf8;
  gfx::PointF m_offsetOrigin;
};

} // anonymous namespace

void SpriteTextBlob::Run::add(const glyph_t glyph, const gfx::PointF& pos, const uint32_t cluster)
{
  glyphs.push_back(glyph);
  positions.push_back(pos);
  clusters.push_back(cluster);
}

void SpriteTextBlob::Run::clear()
{
  subBlob.reset();
  utf8Range.begin = utf8Range.end;
  glyphs.clear();
  positions.clear();
  clusters.clear();
}

void SpriteTextBlob::visitRuns(const RunVisitor& visitor)
{
  RunInfo info;
  info.font = m_font;
  for (Run& run : m_runs) {
    if (run.subBlob) {
      run.subBlob->visitRuns(visitor);
      continue;
    }

    info.utf8Range = run.utf8Range;
    info.glyphCount = run.glyphs.size();
    info.glyphs = run.glyphs.data();
    info.positions = run.positions.data();
    info.clusters = run.clusters.data();
    visitor(info);
  }
}

TextBlobRef SpriteTextBlob::Make(const FontRef& font, const std::string& text)
{
  ASSERT(font);
  ASSERT(font->type() == FontType::SpriteSheet);
  ASSERT(dynamic_cast<SpriteSheetFont*>(font.get()));

  const auto* spriteFont = static_cast<const SpriteSheetFont*>(font.get());

  Runs runs;
  Run run;

  gfx::Rect textBounds;
  gfx::PointF pos(0.0f, 0.0f);
  base::utf8_decode decode(text);
  while (true) {
    const int i = decode.pos() - text.begin();
    const codepoint_t chr = decode.next();
    run.utf8Range.end = i;
    if (chr == 0)
      break;

    // Ignore code point that are not present in the font.
    const glyph_t glyph = spriteFont->codePointToGlyph(chr);
    if (glyph == 0)
      continue;

    gfx::Rect glyphBounds = spriteFont->getGlyphBounds(glyph);
    if (glyphBounds.isEmpty())
      continue;

    run.add(glyph, pos, i - run.utf8Range.begin);

    glyphBounds.offset(pos);
    textBounds |= glyphBounds;
    pos.x += glyphBounds.w;
  }

  if (!run.empty())
    runs.push_back(run);

  return base::make_ref<SpriteTextBlob>(textBounds, font, std::move(runs));
}

} // namespace text
