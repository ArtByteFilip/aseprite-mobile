// LAF Text Library
// Copyright (C) 2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_SPRITE_SHEET_TYPEFACE_H_INCLUDED
#define LAF_TEXT_SPRITE_SHEET_TYPEFACE_H_INCLUDED
#pragma once

#include "base/ref.h"
#include "os/surface.h"
#include "text/typeface.h"

namespace text {

class SpriteSheetTypeface : public Typeface {
public:
  SpriteSheetTypeface() {}

  static base::Ref<SpriteSheetTypeface> FromFile(const char* filename);

  std::string familyName() const override;
  FontStyle fontStyle() const override;

  float defaultSize() const { return m_defaultSize; }
  int maxScale() const { return m_maxScale; }
  void setMaxScale(const int maxScale) { m_maxScale = maxScale; }

  os::Surface* sheetSurface() const { return m_sheet.get(); }
  const std::vector<gfx::Rect>& glyphs() { return m_glyphs; }

private:
  bool fromFile(const char* filename);
  bool findGlyph(const os::Surface* sur,
                 int width,
                 int height,
                 gfx::Rect& bounds,
                 gfx::Rect& glyphBounds);

  os::SurfaceRef m_sheet;
  std::vector<gfx::Rect> m_glyphs;
  float m_defaultSize = 0.0f;

  // It can be greater than zero when we've found a maximum supported
  // size for this font, e.g. because the current computer doesn't
  // have enough memory to handle.
  int m_maxScale = 0;
};

} // namespace text

#endif
