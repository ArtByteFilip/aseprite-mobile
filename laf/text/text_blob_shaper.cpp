// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "text/text_blob.h"

#include "text/font.h"
#include "text/sprite_text_blob.h"

#if LAF_SKIA
  #include "text/skia_text_blob.h"
#endif

namespace text {

TextBlobRef TextBlob::MakeWithShaper(const FontMgrRef& fontMgr,
                                     const FontRef& font,
                                     const std::string& text,
                                     TextBlob::RunHandler* handler,
                                     const ShaperFeatures features)
{
  ASSERT(font);
  switch (font->type()) {
    case FontType::SpriteSheet: return SpriteTextBlob::MakeWithShaper(fontMgr, font, text, handler);

    case FontType::FreeType:
      ASSERT(false); // TODO impl
      return nullptr;

#if LAF_SKIA
    case FontType::Native:
      return SkiaTextBlob::MakeWithShaper(fontMgr, font, text, handler, features);
#endif

    default: return nullptr;
  }
}

} // namespace text
