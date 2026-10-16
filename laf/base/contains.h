// LAF Base Library
// Copyright (c) 2025 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_CONTAINS_H_INCLUDED
#define BASE_CONTAINS_H_INCLUDED
#pragma once

namespace base {

// Returns true if some element of the given "container" matches (is
// equal to) the specified "element" using a linear search. It's a
// shame that C++ never introduced this as a member for all
// containers. C++20 introduced std::set/map::contains() but not a
// std::vector::contains(), which is the most common container we use.
template<typename T>
bool contains(const T& container, typename T::const_reference element)
{
  for (auto it = container.begin(), end = container.end(); it != end; ++it) {
    if (*it == element)
      return true;
  }
  return false;
}

} // namespace base

#endif
