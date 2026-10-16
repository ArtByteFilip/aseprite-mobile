// LAF Text Library
// Copyright (c) 2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FONT_HINTING_H_INCLUDED
#define LAF_TEXT_FONT_HINTING_H_INCLUDED
#pragma once

#include "base/ints.h"

namespace text {

enum class FontHinting : uint8_t {
  None,
  Slight,
  Normal,
  Full,
};

} // namespace text

#endif
