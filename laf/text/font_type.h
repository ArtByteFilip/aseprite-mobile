// LAF Text Library
// Copyright (c) 2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FONT_TYPE_H_INCLUDED
#define LAF_TEXT_FONT_TYPE_H_INCLUDED
#pragma once

#include "base/ints.h"

namespace text {

enum class FontType : uint8_t {
  Unknown,
  SpriteSheet, // SpriteSheet
  FreeType,    // FreeType
  Native,      // Skia
};

} // namespace text

#endif
