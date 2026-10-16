// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_SPRITE_TEXT_BLOB_H_INCLUDED
#define LAF_SPRITE_TEXT_BLOB_H_INCLUDED
#pragma once

#include "gfx/point.h"
#include "text/text_blob.h"

#include <vector>

namespace text {

// A TextBlob created from a SpriteSheetFont. It can include sub-runs
// created using fallback fonts TextBlobs (i.e. SkiaTextBlob) when a
// sequence of non-existent codepoints in the SpriteSheetFont is
// present.
class SpriteTextBlob : public TextBlob {
public:
  struct Run {
    TextBlobRef subBlob;

    Utf8Range utf8Range;
    std::vector<glyph_t> glyphs;
    std::vector<gfx::PointF> positions;
    std::vector<uint32_t> clusters;

    size_t size() const { return glyphs.size(); }
    bool empty() const { return glyphs.empty(); }

    void add(glyph_t glyph, const gfx::PointF& pos, uint32_t cluster);
    void clear();
  };
  using Runs = std::vector<Run>;

  SpriteTextBlob(const gfx::RectF& bounds, const FontRef& font, Runs&& runs)
    : TextBlob(bounds)
    , m_font(font)
    , m_runs(std::move(runs))
  {
  }
  ~SpriteTextBlob() {}

  void visitRuns(const RunVisitor& visitor) override;

  static TextBlobRef Make(const FontRef& font, const std::string& text);

  static TextBlobRef MakeWithShaper(const FontMgrRef& fontMgr,
                                    const FontRef& font,
                                    const std::string& text,
                                    TextBlob::RunHandler* handler);

  const FontRef& font() const { return m_font; }
  const Runs& runs() const { return m_runs; }

private:
  FontRef m_font;
  Runs m_runs;
};

} // namespace text

#endif
