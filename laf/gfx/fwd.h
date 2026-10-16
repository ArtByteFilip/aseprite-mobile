// LAF Gfx Library
// Copyright (C) 2024  Igara Studio S.A.
// Copyright (C) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef GFX_FWD_H_INCLUDED
#define GFX_FWD_H_INCLUDED
#pragma once

namespace gfx {

template<typename T>
class BorderT;
template<typename T>
class ClipT;
template<typename T>
class PointT;
template<typename T>
class RectT;
template<typename T>
class SizeT;

using Border = BorderT<int>;
using BorderF = BorderT<float>;
using Clip = ClipT<int>;
using ClipF = ClipT<float>;
using Point = PointT<int>;
using PointF = PointT<float>;
using Rect = RectT<int>;
using RectF = RectT<float>;
using Size = SizeT<int>;
using SizeF = SizeT<float>;

class Region;

} // namespace gfx

#endif
