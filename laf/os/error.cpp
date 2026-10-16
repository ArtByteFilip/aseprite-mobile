// LAF OS Library
// Copyright (c) 2024-2025  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/error.h"

#include "base/string.h"
#include "os/system.h"

#if LAF_WINDOWS
  #include <windows.h>
#endif

namespace os {

void error_message(const char* msg)
{
#if LAF_WINDOWS
  if (msg)
    MessageBoxW(nullptr, base::from_utf8(msg).c_str(), L"Error", MB_OK | MB_ICONERROR);
#endif

  fputs(msg, stderr);
}

} // namespace os
