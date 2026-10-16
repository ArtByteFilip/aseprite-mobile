// LAF Text Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FWD_H_INCLUDED
#define LAF_TEXT_FWD_H_INCLUDED
#pragma once

#include "base/codepoint.h"
#include "base/glyph.h"
#include "base/ref.h"

namespace text {

using codepoint_t = base::codepoint_t;
using glyph_t = base::glyph_t;

class DrawTextDelegate;

class Font;
using FontRef = base::Ref<Font>;

enum class FontType : uint8_t;

struct FontMetrics;

class FontMgr;
using FontMgrRef = base::Ref<FontMgr>;

class FontStyle;
using FontStyleRef = base::Ref<FontStyle>;

class FontStyleSet;
using FontStyleSetRef = base::Ref<FontStyleSet>;

struct ShaperFeatures;

class TextBlob;
using TextBlobRef = base::Ref<TextBlob>;

class Typeface;
using TypefaceRef = base::Ref<Typeface>;

class SpriteSheetFont;
class SpriteSheetTypeface;

} // namespace text

#endif
