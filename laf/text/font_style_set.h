// LAF Text Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FONT_STYLE_SET_H_INCLUDED
#define LAF_TEXT_FONT_STYLE_SET_H_INCLUDED
#pragma once

#include "text/fwd.h"

namespace text {

class FontStyleSet : public base::RefCount {
protected:
  virtual ~FontStyleSet() {}

public:
  virtual int count() = 0;
  virtual void getStyle(int index, FontStyle& style, std::string& name) = 0;
  virtual TypefaceRef typeface(int index) = 0;
  virtual TypefaceRef matchStyle(const FontStyle& style) = 0;
};

} // namespace text

#endif
