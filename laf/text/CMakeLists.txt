# LAF Text
# Copyright (C) 2024-2025  Igara Studio S.A.

######################################################################

add_library(laf-text
  draw_text.cpp
  draw_text_shaper.cpp
  font_mgr.cpp
  sprite_sheet_font.cpp
  sprite_sheet_typeface.cpp
  sprite_text_blob.cpp
  sprite_text_blob_shaper.cpp
  text_blob.cpp
  text_blob_shaper.cpp)

target_link_libraries(laf-text laf-os laf-gfx laf-base)

if(FREETYPE_LIBRARIES AND HARFBUZZ_LIBRARIES)
  target_link_libraries(laf-text laf-ft)
  target_sources(laf-text PRIVATE
    freetype_font.cpp)
endif()

if(LAF_BACKEND STREQUAL "skia")
  target_sources(laf-text PRIVATE
    skia_font.cpp
    skia_font_mgr.cpp
    skia_text_blob.cpp
    skia_text_blob_shaper.cpp)
  target_link_libraries(laf-text
    skia skshaper)
else()
  target_sources(laf-text PRIVATE
    empty_font_mgr.cpp)
endif()

if(LAF_WITH_TESTS)
  laf_find_tests(. laf-text)
endif()
