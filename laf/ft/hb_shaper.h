// LAF FreeType Wrapper
// Copyright (c) 2020-2024 Igara Studio S.A.
// Copyright (c) 2017 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef FT_HB_SHAPER_H_INCLUDED
#define FT_HB_SHAPER_H_INCLUDED
#pragma once

#include "base/glyph.h"
#include "base/utf8_decode.h"
#include "ft/hb_face.h"

#include <vector>

namespace ft {

template<typename HBFace>
class HBShaper {
public:
  HBShaper(HBFace& face, const std::string& str) : m_face(face)
  {
    base::utf8_decode decode(str);
    if (decode.is_end())
      return;

    hb_buffer_t* buf = hb_buffer_create();
    hb_buffer_t* chrBuf = hb_buffer_create();
    hb_script_t script = HB_SCRIPT_UNKNOWN;

    const auto begin = str.begin();
    while (true) {
      const auto pos = decode.pos();
      const base::codepoint_t chr = decode.next();
      if (!chr)
        break;

      // Get the script of the next character in *it
      hb_buffer_set_content_type(chrBuf, HB_BUFFER_CONTENT_TYPE_UNICODE);
      hb_buffer_add(chrBuf, chr, 0);
      hb_buffer_guess_segment_properties(chrBuf);
      hb_script_t newScript = hb_buffer_get_script(chrBuf);
      hb_buffer_reset(chrBuf);

      if (newScript && script != newScript) {
        addBuffer(buf, script);
        hb_buffer_reset(buf);
        script = newScript;
      }

      hb_buffer_add(buf, chr, pos - begin);
    }
    addBuffer(buf, script);

    hb_buffer_destroy(buf);
    hb_buffer_destroy(chrBuf);
  }

  base::codepoint_t next()
  {
    if (++m_index < m_glyphCount)
      return m_codePoints[m_index];
    return 0;
  }

  base::codepoint_t unicodeChar() const
  {
    if (m_index >= 0 && m_index < m_codePoints.size())
      return m_codePoints[m_index];
    else
      return 0;
  }

  int charIndex() const { return m_glyphInfo[m_index].cluster; }

  base::glyph_t glyphIndex() const
  {
    // After shaping the "codepoint" field is the glyph index.
    return m_glyphInfo[m_index].codepoint;
  }

  void glyphOffsetXY(Glyph* glyph)
  {
    glyph->x += m_glyphPos[m_index].x_offset / 64.0;
    glyph->y += m_glyphPos[m_index].y_offset / 64.0;
  }

  void glyphAdvanceXY(const Glyph* glyph, double& x, double& y)
  {
    x += m_glyphPos[m_index].x_advance / 64.0;
    y += m_glyphPos[m_index].y_advance / 64.0;
  }

private:
  void addBuffer(hb_buffer_t* buf, hb_script_t script)
  {
    if (hb_buffer_get_length(buf) == 0)
      return;

    // Just in case we're compiling with an old harfbuzz version
#ifdef HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS
    hb_buffer_set_cluster_level(buf, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
#endif
    hb_buffer_set_content_type(buf, HB_BUFFER_CONTENT_TYPE_UNICODE);
    hb_buffer_set_script(buf, script);
    hb_buffer_set_direction(buf, hb_script_get_horizontal_direction(script));

    // Update code points array
    unsigned int count;
    hb_glyph_info_t* info;
    {
      info = hb_buffer_get_glyph_infos(buf, &count);
      const auto start = m_codePoints.size();
      m_codePoints.resize(start + count);
      for (auto i = 0; i < count; ++i) {
        m_codePoints[start + i] = info[i].codepoint;
      }
    }

    // Shape text
    hb_shape(m_face.font(), buf, nullptr, 0);

    // Update glyph info/pos arrays.
    info = hb_buffer_get_glyph_infos(buf, &count);
    auto pos = hb_buffer_get_glyph_positions(buf, &count);

    m_glyphCount += count;
    const auto start = m_glyphInfo.size();
    m_glyphInfo.resize(m_glyphCount);
    m_glyphPos.resize(m_glyphCount);
    for (unsigned int i = 0; i < count; ++i) {
      m_glyphInfo[start + i] = info[i];
      m_glyphPos[start + i] = pos[i];
    }
  }

  HBFace& m_face;
  std::vector<base::codepoint_t> m_codePoints;
  std::vector<hb_glyph_info_t> m_glyphInfo;
  std::vector<hb_glyph_position_t> m_glyphPos;
  int m_glyphCount = 0;
  int m_index = -1;
};

} // namespace ft

#endif
