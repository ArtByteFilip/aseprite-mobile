// LAF Base Library
// Copyright (c) 2024 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_GLYPH_H_INCLUDED
#define BASE_GLYPH_H_INCLUDED
#pragma once

#include "base/ints.h"

namespace base {

// A font file is a set of glyphs that represent one (or a
// combination) of code points (base::codepoint_t). This value is
// like a reference/index in a font glyph table.
using glyph_t = uint16_t;

} // namespace base

#endif
