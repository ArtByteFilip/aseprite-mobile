// LAF Base Library
// Copyright (c) 2001-2016 David Capello
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef BASE_LAUNCHER_H_INCLUDED
#define BASE_LAUNCHER_H_INCLUDED
#pragma once

#include <string>

namespace base { namespace launcher {

bool open_url(const std::string& url);
bool open_file(const std::string& file);
bool open_folder(const std::string& file);

#if LAF_MOBILE
// Function to open files/URLs with other apps on mobile platforms (it's set by
// laf-os as it requires JNI calls to the activity).
using OpenFileHandler = bool (*)(const std::string& file);
void set_open_file_handler(OpenFileHandler handler);
#endif

}} // namespace base::launcher

#endif
