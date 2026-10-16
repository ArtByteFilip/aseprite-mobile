// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "os/android/android_app.h"

#include "base/launcher.h"
#include "os/android/system.h"
#include "os/android/window.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/mobile.h"

#include <android/configuration.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <jni.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <unistd.h>
#include <mutex>

#define LOG_TAG "aseprite"
#define ALOG(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace os {

namespace {

android_app* g_app = nullptr;
JNIEnv* g_env = nullptr;
ANativeWindow* g_nativeWindow = nullptr;
std::atomic<bool> g_destroyRequested(false);
int g_dpi = 160;

std::mutex g_pendingFilesMutex;
std::vector<std::string> g_pendingFiles;


void update_configuration(android_app* app)
{
  if (app->config) {
    const int density = AConfiguration_getDensity(app->config);
    if (density > 0 && density != ACONFIGURATION_DENSITY_NONE &&
        density != ACONFIGURATION_DENSITY_ANY) {
      g_dpi = density;
    }
  }
}

void queue_simple_event(Event::Type type)
{
  Event ev;
  ev.setType(type);
  if (WindowAndroid* window = WindowAndroid::instance())
    window->queueEvent(ev);
  else
    EventQueue::instance()->queueEvent(ev);
}

void handle_cmd(android_app* app, int32_t cmd)
{
  WindowAndroid* window = WindowAndroid::instance();
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      g_nativeWindow = app->window;
      if (window)
        window->onNativeWindowChanged(g_nativeWindow);
      break;

    case APP_CMD_TERM_WINDOW:
      g_nativeWindow = nullptr;
      if (window)
        window->onNativeWindowChanged(nullptr);
      break;

    case APP_CMD_WINDOW_RESIZED:
    case APP_CMD_CONTENT_RECT_CHANGED:
      if (window)
        window->checkSize();
      break;

    case APP_CMD_CONFIG_CHANGED:
      update_configuration(app);
      if (window)
        window->checkSize();
      break;

    case APP_CMD_WINDOW_REDRAW_NEEDED:
      if (window)
        window->redrawAll();
      break;

    case APP_CMD_GAINED_FOCUS: queue_simple_event(Event::WindowEnter); break;

    case APP_CMD_LOST_FOCUS:
      if (window)
        window->releaseAllKeys();
      queue_simple_event(Event::WindowLeave);
      break;

    case APP_CMD_PAUSE: call_app_pause_handler(); break;

    case APP_CMD_DESTROY: g_destroyRequested = true; break;
  }
}

int32_t handle_input(android_app* app, AInputEvent* event)
{
  if (WindowAndroid* window = WindowAndroid::instance())
    return window->handleInputEvent(event);
  return 0;
}

jobject activity_object()
{
  return (g_app && g_app->activity ? g_app->activity->clazz : nullptr);
}

// Calls a method of the Java AsepriteActivity instance
template<typename Func>
void with_activity_method(const char* name, const char* sig, Func&& func)
{
  if (!g_env || !activity_object())
    return;

  jclass cls = g_env->GetObjectClass(activity_object());
  if (!cls)
    return;
  jmethodID method = g_env->GetMethodID(cls, name, sig);
  if (method)
    func(g_env, activity_object(), method);
  if (g_env->ExceptionCheck()) {
    g_env->ExceptionDescribe();
    g_env->ExceptionClear();
  }
  g_env->DeleteLocalRef(cls);
}

void flush_pending_files()
{
  WindowAndroid* window = WindowAndroid::instance();
  if (!window)
    return;

  std::vector<std::string> files;
  {
    std::lock_guard lock(g_pendingFilesMutex);
    if (g_pendingFiles.empty())
      return;
    files.swap(g_pendingFiles);
  }

  Event ev;
  ev.setType(Event::DropFiles);
  ev.setFiles(base::paths(files.begin(), files.end()));
  window->queueEvent(ev);
}

} // anonymous namespace

android_app* android_get_app()
{
  return g_app;
}

void android_set_app(android_app* app)
{
  g_app = app;
  app->onAppCmd = handle_cmd;
  app->onInputEvent = handle_input;

  if (app->activity->vm->AttachCurrentThread(&g_env, nullptr) != JNI_OK)
    g_env = nullptr;

  update_configuration(app);

  base::launcher::set_open_file_handler(
    [](const std::string& file) -> bool { return android_open_url(file); });
}

ANativeWindow* android_native_window()
{
  return g_nativeWindow;
}

void android_process_events(int timeoutMs)
{
  if (!g_app)
    return;

  int timeout = timeoutMs;
  while (true) {
    int events = 0;
    android_poll_source* source = nullptr;
    const int id = ALooper_pollOnce(timeout, nullptr, &events, (void**)&source);
    if (id == ALOOPER_POLL_TIMEOUT || id == ALOOPER_POLL_ERROR || id == ALOOPER_POLL_WAKE)
      break;
    if (id >= 0 && source)
      source->process(g_app, source);

    // Don't wait for more events after receiving the first one
    timeout = 0;

    if (g_app->destroyRequested)
      g_destroyRequested = true;
  }

  // The activity is being destroyed (e.g. the system is closing the
  // app), we cannot restart the app state in the same process.
  if (g_destroyRequested && g_nativeWindow == nullptr && WindowAndroid::instance()) {
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "Activity destroyed, exiting");
    // Don't call static destructors (some objects could be in use
    // by background threads)
    _exit(0);
  }

  flush_pending_files();
}

bool android_is_destroy_requested()
{
  return g_destroyRequested || (g_app && g_app->destroyRequested);
}

int android_screen_dpi()
{
  return g_dpi;
}

int default_screen_scale()
{
  int scale = std::clamp((g_dpi + 80) / 160, 1, 4);
  if (g_nativeWindow) {
    const int w = ANativeWindow_getWidth(g_nativeWindow);
    const int h = ANativeWindow_getHeight(g_nativeWindow);
    const int minSide = std::min(w, h);
    // Keep at least 400 pixels of UI space in the smallest side
    while (scale > 1 && minSide / scale < 400)
      --scale;
  }
  return scale;
}

void show_soft_keyboard(bool state)
{
  with_activity_method("showSoftKeyboard", "(Z)V", [state](JNIEnv* env, jobject obj, jmethodID m) {
    env->CallVoidMethod(obj, m, (jboolean)state);
  });
}

int android_get_unicode_char(int keyCode, int metaState)
{
  int result = 0;
  with_activity_method("getUnicodeChar", "(II)I", [&](JNIEnv* env, jobject obj, jmethodID m) {
    result = env->CallIntMethod(obj, m, (jint)keyCode, (jint)metaState);
  });
  return result;
}

void android_haptic_feedback()
{
  with_activity_method("hapticFeedback", "()V", [](JNIEnv* env, jobject obj, jmethodID m) {
    env->CallVoidMethod(obj, m);
  });
}

bool android_open_url(const std::string& url)
{
  bool result = false;
  with_activity_method("openUrl", "(Ljava/lang/String;)Z", [&](JNIEnv* env, jobject obj, jmethodID m) {
    jstring jurl = env->NewStringUTF(url.c_str());
    result = env->CallBooleanMethod(obj, m, jurl);
    env->DeleteLocalRef(jurl);
  });
  return result;
}

bool set_clipboard_text(const std::string& text)
{
  bool result = false;
  with_activity_method("setClipboardText",
                       "(Ljava/lang/String;)Z",
                       [&](JNIEnv* env, jobject obj, jmethodID m) {
                         jstring jtext = env->NewStringUTF(text.c_str());
                         result = env->CallBooleanMethod(obj, m, jtext);
                         env->DeleteLocalRef(jtext);
                       });
  return result;
}

bool get_clipboard_text(std::string& text)
{
  bool result = false;
  with_activity_method("getClipboardText",
                       "()Ljava/lang/String;",
                       [&](JNIEnv* env, jobject obj, jmethodID m) {
                         auto jtext = (jstring)env->CallObjectMethod(obj, m);
                         if (jtext) {
                           const char* utf8 = env->GetStringUTFChars(jtext, nullptr);
                           if (utf8) {
                             text = utf8;
                             result = true;
                             env->ReleaseStringUTFChars(jtext, utf8);
                           }
                           env->DeleteLocalRef(jtext);
                         }
                       });
  return result;
}

std::vector<std::string> android_take_pending_files()
{
  std::lock_guard lock(g_pendingFilesMutex);
  std::vector<std::string> files;
  files.swap(g_pendingFiles);
  return files;
}

} // namespace os

//////////////////////////////////////////////////////////////////////
// Native methods called from AsepriteActivity.java (UI thread)

extern "C" {

JNIEXPORT void JNICALL Java_org_aseprite_android_AsepriteActivity_nativeOnTextInput(JNIEnv* env,
                                                                                     jclass,
                                                                                     jstring text)
{
  if (!text)
    return;
  const char* utf8 = env->GetStringUTFChars(text, nullptr);
  if (!utf8)
    return;
  const std::string str(utf8);
  env->ReleaseStringUTFChars(text, utf8);

  // Queue a callback to process the text in the native thread
  os::Event ev;
  ev.setType(os::Event::Callback);
  ev.setCallback([str] {
    if (os::WindowAndroid* window = os::WindowAndroid::instance())
      window->queueTextInput(str);
  });
  os::EventQueue::instance()->queueEvent(ev);
}

JNIEXPORT void JNICALL Java_org_aseprite_android_AsepriteActivity_nativeOnBackPressed(JNIEnv*, jclass)
{
  // The Android back button/gesture works as the Esc key
  os::Event ev;
  ev.setType(os::Event::Callback);
  ev.setCallback([] {
    os::WindowAndroid* window = os::WindowAndroid::instance();
    if (!window)
      return;
    os::Event key;
    key.setScancode(os::kKeyEsc);
    key.setModifiers(os::kKeyNoneModifier);
    key.setUnicodeChar(0);
    key.setType(os::Event::KeyDown);
    window->queueEvent(key);
    key.setType(os::Event::KeyUp);
    window->queueEvent(key);
  });
  os::EventQueue::instance()->queueEvent(ev);
}

JNIEXPORT void JNICALL Java_org_aseprite_android_AsepriteActivity_nativeOnOpenFile(JNIEnv* env,
                                                                                   jclass,
                                                                                   jstring path)
{
  if (!path)
    return;
  const char* utf8 = env->GetStringUTFChars(path, nullptr);
  if (!utf8)
    return;
  {
    std::lock_guard lock(os::g_pendingFilesMutex);
    os::g_pendingFiles.push_back(utf8);
  }
  env->ReleaseStringUTFChars(path, utf8);

  if (os::g_app && os::g_app->looper)
    ALooper_wake(os::g_app->looper);
}

} // extern "C"
