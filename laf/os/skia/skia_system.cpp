// LAF OS Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/skia/skia_system.h"

namespace os {

SystemRef System::makeSkia()
{
  return make_ref<SkiaSystem>();
}

} // namespace os
