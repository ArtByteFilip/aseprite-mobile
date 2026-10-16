// LAF OS Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef TEXT_FONT_METRICS_H_INCLUDED
#define TEXT_FONT_METRICS_H_INCLUDED
#pragma once

namespace text {

// Based on SkFontMetrics
struct FontMetrics {
  float top = 0.0f; // Greatest extent above origin of any glyph bounding box, typically negative;
                    // deprecated with variable fonts
  float ascent = 0.0f;       // Distance to reserve above baseline, typically negative
  float descent = 0.0f;      // Distance to reserve below baseline, typically positive
  float bottom = 0.0f;       // Greatest extent below origin of any glyph bounding box, typically
                             // positive; deprecated with variable fonts
  float leading = 0.0f;      // Distance to add between lines, typically positive or zero
  float avgCharWidth = 0.0f; // Average character width, zero if unknown
  float maxCharWidth = 0.0f; // Maximum character width, zero if unknown
  float xMin = 0.0f;      // Greatest extent to left of origin of any glyph bounding box, typically
                          // negative; deprecated with variable fonts
  float xMax = 0.0f;      // Greatest extent to right of origin of any glyph bounding box, typically
                          // positive; deprecated with variable fonts
  float xHeight = 0.0f;   // Height of lower-case 'x', zero if unknown, typically negative
  float capHeight = 0.0f; // Height of an upper-case letter, zero if unknown, typically negative
  float underlineThickness = 0.0f; // Underline thickness
  float underlinePosition = 0.0f;  // Distance from baseline to top of stroke, typically positive
  float strikeoutThickness = 0.0f; // Strikeout thickness
  float strikeoutPosition = 0.0f;  // Distance from baseline to bottom of stroke, typically negative
};

} // namespace text

#endif
