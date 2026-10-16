// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "base/memory.h"
#include "os/android/android_app.h"

#include <android/log.h>
#include <android_native_app_glue.h>

#include <cstdlib>
#include <unistd.h>
#include <string>
#include <vector>

extern int app_main(int argc, char* argv[]);

// Entry point called by the native_app_glue in a new thread.
void android_main(android_app* app)
{
  os::android_set_app(app);

  // Wait until we have a native window, so the os::Window can be
  // created with the real screen size.
  while (!os::android_native_window() && !os::android_is_destroy_requested())
    os::android_process_events(-1);

  if (os::android_is_destroy_requested())
    _exit(0);

  std::vector<std::string> args;
  args.push_back("aseprite");
  for (const auto& fn : os::android_take_pending_files())
    args.push_back(fn);

  std::vector<char*> argv;
  for (const auto& arg : args)
    argv.push_back(base_strdup(arg.c_str()));
  argv.push_back(nullptr);

  __android_log_print(ANDROID_LOG_INFO, "aseprite", "Starting app_main()");
  const int result = app_main(int(args.size()), argv.data());
  __android_log_print(ANDROID_LOG_INFO, "aseprite", "app_main() finished (%d)", result);

  // The app uses a lot of global/static state that cannot be
  // re-initialized if the activity is created again in the same
  // process, so we close the activity and terminate the process.
  ANativeActivity_finish(app->activity);
  // Static destructors are not called as some background threads
  // could still be using global objects.
  _exit(result);
}
