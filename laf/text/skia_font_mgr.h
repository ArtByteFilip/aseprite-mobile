// LAF Text Library
// Copyright (c) 2019-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef LAF_TEXT_SKIA_FONT_MGR_INCLUDED
#define LAF_TEXT_SKIA_FONT_MGR_INCLUDED
#pragma once

#include "text/font_mgr.h"
#include "text/font_style.h"
#include "text/font_style_set.h"
#include "text/typeface.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"

#include <mutex>

namespace text {

class SkiaFontStyleSet;

class SkiaTypeface : public Typeface {
public:
  SkiaTypeface(sk_sp<SkTypeface> skTypeface, SkiaFontStyleSet* owner);

  std::string familyName() const override;
  FontStyle fontStyle() const override;

  sk_sp<SkTypeface> skTypeface() const { return m_skTypeface; }

private:
  sk_sp<SkTypeface> m_skTypeface;
  SkiaFontStyleSet* m_owner;
};

class SkiaFontStyleSet : public FontStyleSet {
public:
  SkiaFontStyleSet(sk_sp<SkFontStyleSet> set);

  int count() override;
  void getStyle(int index, FontStyle& style, std::string& name) override;
  TypefaceRef typeface(int index) override;
  TypefaceRef matchStyle(const FontStyle& style) override;

  struct LockSet;

private:
  std::mutex m_mutex;
  sk_sp<SkFontStyleSet> m_skSet;
};

class SkiaFontMgr : public FontMgr {
public:
  SkiaFontMgr();
  ~SkiaFontMgr();

  FontRef loadTrueTypeFont(const char* filename, float size) override;

  FontRef defaultFont(float size) const override;
  FontRef makeFont(const TypefaceRef& typeface) override;
  FontRef makeFont(const TypefaceRef& typeface, float size) override;

  int countFamilies() const override;
  std::string familyName(int i) const override;
  FontStyleSetRef familyStyleSet(int i) const override;
  FontStyleSetRef matchFamily(const std::string& familyName) const override;

  sk_sp<SkFontMgr> skFontMgr() const { return m_skFontMgr; }

private:
  sk_sp<SkFontMgr> m_skFontMgr;
};

} // namespace text

#endif
