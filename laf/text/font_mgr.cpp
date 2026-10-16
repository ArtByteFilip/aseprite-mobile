// LAF Text Library
// Copyright (C) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/font_mgr.h"

#if LAF_FREETYPE
  #include "ft/lib.h"
  #include "text/freetype_font.h"
#endif
#include "text/sprite_sheet_font.h"
#include "text/sprite_sheet_typeface.h"

namespace text {

FontMgr::FontMgr()
{
}

FontMgr::~FontMgr()
{
}

FontRef FontMgr::loadSpriteSheetFont(const char* filename, float size)
{
  base::Ref<SpriteSheetTypeface> typeface;
  auto it = m_spriteSheetTypefaces.find(filename);
  if (it != m_spriteSheetTypefaces.end())
    typeface = it->second;
  else {
    typeface = SpriteSheetTypeface::FromFile(filename);
    if (!typeface)
      return nullptr;

    // Cache this typeface
    m_spriteSheetTypefaces[filename] = typeface;
  }

  return base::make_ref<SpriteSheetFont>(typeface, size);
}

FontRef FontMgr::loadTrueTypeFont(const char* filename, float size)
{
#if LAF_FREETYPE
  if (!m_ft)
    m_ft.reset(new ft::Lib());
  return FreeTypeFont::LoadFont(*m_ft.get(), filename, size);
#else
  return nullptr;
#endif
}

} // namespace text
