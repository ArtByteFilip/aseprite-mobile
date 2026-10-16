// LAF Text Library
// Copyright (C) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_FONT_MGR_H_INCLUDED
#define LAF_TEXT_FONT_MGR_H_INCLUDED
#pragma once

#include "text/fwd.h"

#include <map>
#include <memory>
#include <string>

namespace ft {
class Lib;
}

namespace text {

class FontMgr : public base::RefCount {
public:
  [[nodiscard]]
  static FontMgrRef Make();

  FontRef loadSpriteSheetFont(const char* filename, float size);
  virtual FontRef loadTrueTypeFont(const char* filename, float size);
  virtual FontRef makeFont(const TypefaceRef& typeface) = 0;
  virtual FontRef makeFont(const TypefaceRef& typeface, float size) = 0;

  virtual FontRef defaultFont(float size = 12) const = 0;
  virtual int countFamilies() const = 0;
  virtual std::string familyName(int index) const = 0;
  virtual FontStyleSetRef familyStyleSet(int index) const = 0;
  virtual FontStyleSetRef matchFamily(const std::string& familyName) const = 0;

protected:
  FontMgr();
  virtual ~FontMgr();

private:
#if LAF_FREETYPE
  std::unique_ptr<ft::Lib> m_ft;
#endif
  std::map<std::string, base::Ref<SpriteSheetTypeface>> m_spriteSheetTypefaces;
};

} // namespace text

#endif
