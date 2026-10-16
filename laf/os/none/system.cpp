// LAF OS Library
// Copyright (c) 2024  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/common/system.h"

namespace os {

class NoneSystem : public CommonSystem {};

SystemRef System::makeNone()
{
  return make_ref<NoneSystem>();
}

} // namespace os
