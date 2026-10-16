// LAF OS Library
// Copyright (C) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/event.h"

#include "base/string.h"

namespace os {

std::string Event::unicodeCharAsUtf8() const
{
  return base::codepoint_to_utf8(m_unicodeChar);
}

} // namespace os
