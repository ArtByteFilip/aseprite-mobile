// LAF Library
// Copyright (c) 2019-2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "os/os.h"
#include "text/text.h"

#include <cassert>
#include <cstdio>

using namespace os;
using namespace text;

static const char* to_str(FontStyle::Weight weight)
{
  switch (weight) {
    case FontStyle::Weight::Invisible:  return "Invisible";
    case FontStyle::Weight::Thin:       return "Thin";
    case FontStyle::Weight::ExtraLight: return "ExtraLight";
    case FontStyle::Weight::Light:      return "Light";
    case FontStyle::Weight::Normal:     return "Normal";
    case FontStyle::Weight::Medium:     return "Medium";
    case FontStyle::Weight::SemiBold:   return "SemiBold";
    case FontStyle::Weight::Bold:       return "Bold";
    case FontStyle::Weight::ExtraBold:  return "ExtraBold";
    case FontStyle::Weight::Black:      return "Black";
    case FontStyle::Weight::ExtraBlack: return "ExtraBlack";
  }
  return "";
}

static const char* to_str(FontStyle::Width width)
{
  switch (width) {
    case FontStyle::Width::UltraCondensed: return "UltraCondensed";
    case FontStyle::Width::ExtraCondensed: return "ExtraCondensed";
    case FontStyle::Width::Condensed:      return "Condensed";
    case FontStyle::Width::SemiCondensed:  return "SemiCondensed";
    case FontStyle::Width::Normal:         return "Normal";
    case FontStyle::Width::SemiExpanded:   return "SemiExpanded";
    case FontStyle::Width::Expanded:       return "Expanded";
    case FontStyle::Width::ExtraExpanded:  return "ExtraExpanded";
    case FontStyle::Width::UltraExpanded:  return "UltraExpanded";
  }
  return "";
}

static const char* to_str(FontStyle::Slant slant)
{
  switch (slant) {
    case FontStyle::Slant::Upright: return "Upright";
    case FontStyle::Slant::Italic:  return "Italic";
    case FontStyle::Slant::Oblique: return "Oblique";
  }
  return "";
}

static void print_set(const std::string& name, FontStyleSet* set)
{
  for (int j = 0; j < set->count(); ++j) {
    FontStyle style;
    std::string styleName;
    set->getStyle(j, style, styleName);
    std::printf(" * %s (%s %s %s)\n",
                name.c_str(),
                to_str(style.weight()),
                to_str(style.width()),
                to_str(style.slant()));
  }
}

int app_main(int argc, char* argv[])
{
  SystemRef system = System::make();
  system->setAppMode(AppMode::CLI);

  FontMgrRef fm = FontMgr::Make();
  if (!fm) {
    std::printf("There is no font manager in your platform\n");
    return 1;
  }

  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      std::string name = argv[i];
      std::printf("Font %s:\n", name.c_str());
      auto set = fm->matchFamily(name);
      if (!set) {
        std::printf("Font family '%s' not found\n", argv[i]);
        return 1;
      }
      print_set(name, set.get());
    }
  }
  // Print all font families
  else {
    std::printf("Font families (%d):\n", fm->countFamilies());
    const int n = fm->countFamilies();
    for (int i = 0; i < n; ++i) {
      std::string name = fm->familyName(i);
      std::printf("%s\n", name.c_str());

      auto fnset = fm->matchFamily(name);
      auto set = fm->familyStyleSet(i);
      assert(fnset->count() == set->count());

      print_set(name, set.get());
    }
  }
  return 0;
}
