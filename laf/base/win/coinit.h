// LAF Base Library
// Copyright (c) 2024 Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_WIN_COINIT_H_INCLUDED
#define BASE_WIN_COINIT_H_INCLUDED
#pragma once

#if !LAF_WINDOWS
  #error This header file can be used only on Windows platform
#endif

#include <objbase.h>

namespace base {

// Successful calls to CoInitialize() (S_OK or S_FALSE) must match
// the calls to CoUninitialize().
// From:
//   https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize#remarks
struct CoInit {
  HRESULT hr;
  CoInit() { hr = CoInitialize(nullptr); }
  ~CoInit()
  {
    if (hr == S_OK || hr == S_FALSE)
      CoUninitialize();
  }
};

} // namespace base

#endif
