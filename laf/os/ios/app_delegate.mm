// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "base/launcher.h"
#include "base/memory.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/ios/ios_app.h"
#include "os/ios/view.h"
#include "os/ios/window.h"
#include "os/mobile.h"

#include <UIKit/UIKit.h>

#include <algorithm>
#include <cmath>
#include <mutex>
#include <unistd.h>

extern int app_main(int argc, char* argv[]);

namespace {

AsepriteViewController* g_viewController = nil;
bool g_appMainStarted = false;
std::mutex g_pendingFilesMutex;
std::vector<std::string> g_pendingFiles;

void start_app_main()
{
  if (g_appMainStarted)
    return;
  g_appMainStarted = true;

  // Run app_main() from the main thread, but after the current UIKit
  // event (scene connection) is completely processed. The app has its
  // own event loop that runs the CFRunLoop (see ios_process_events()).
  dispatch_async(dispatch_get_main_queue(), ^{
    std::vector<std::string> args;
    args.push_back("aseprite");
    for (const auto& fn : os::ios_take_pending_files())
      args.push_back(fn);

    std::vector<char*> argv;
    for (const auto& arg : args)
      argv.push_back(base_strdup(arg.c_str()));
    argv.push_back(nullptr);

    const int result = app_main(int(args.size()), argv.data());

    // The user closed the app (File > Exit)
    _exit(result);
  });
}

// Copies a file received from other app to the Documents/Inbox
// folder (so the user can find it later in the Files app).
std::string import_url(NSURL* url)
{
  if (!url.isFileURL)
    return std::string();

  const BOOL scoped = [url startAccessingSecurityScopedResource];

  NSString* docs =
    NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
  NSString* path = url.path;
  std::string result;

  if ([path hasPrefix:docs]) {
    // The file is already inside our Documents folder
    result = [path UTF8String];
  }
  else {
    NSString* inbox = [docs stringByAppendingPathComponent:@"Inbox"];
    [[NSFileManager defaultManager] createDirectoryAtPath:inbox
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    NSString* dst = [inbox stringByAppendingPathComponent:url.lastPathComponent];
    [[NSFileManager defaultManager] removeItemAtPath:dst error:nil];
    if ([[NSFileManager defaultManager] copyItemAtPath:path toPath:dst error:nil])
      result = [dst UTF8String];
  }

  if (scoped)
    [url stopAccessingSecurityScopedResource];
  return result;
}

void open_urls(NSSet<UIOpenURLContext*>* contexts)
{
  for (UIOpenURLContext* ctx in contexts) {
    std::string fn = import_url(ctx.URL);
    if (!fn.empty())
      os::ios_add_pending_file(fn);
  }
  os::ios_wakeup();
}

} // anonymous namespace

AsepriteViewController* ios_main_view_controller()
{
  return g_viewController;
}

//////////////////////////////////////////////////////////////////////
// Scene delegate

@interface AsepriteSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@implementation AsepriteSceneDelegate

- (void)scene:(UIScene*)scene
  willConnectToSession:(UISceneSession*)session
               options:(UISceneConnectionOptions*)connectionOptions
{
  UIWindowScene* windowScene = (UIWindowScene*)scene;
  self.window = [[UIWindow alloc] initWithWindowScene:windowScene];

  g_viewController = [[AsepriteViewController alloc] init];
  self.window.rootViewController = g_viewController;
  [self.window makeKeyAndVisible];

  open_urls(connectionOptions.URLContexts);
  start_app_main();
}

- (void)scene:(UIScene*)scene openURLContexts:(NSSet<UIOpenURLContext*>*)URLContexts
{
  open_urls(URLContexts);
}

- (void)sceneWillResignActive:(UIScene*)scene
{
  if (os::WindowIOS* window = os::WindowIOS::instance())
    window->releaseAllKeys();
}

- (void)sceneDidEnterBackground:(UIScene*)scene
{
  // Save backups/preferences (iOS can kill the app in background)
  os::call_app_pause_handler();
  os::ios_wakeup();
}

@end

//////////////////////////////////////////////////////////////////////
// App delegate

@interface AsepriteAppDelegate : UIResponder <UIApplicationDelegate>
@end

@implementation AsepriteAppDelegate

- (BOOL)application:(UIApplication*)application
  didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
  base::launcher::set_open_file_handler(
    [](const std::string& file) -> bool { return os::ios_open_url(file); });
  return YES;
}

- (UISceneConfiguration*)application:(UIApplication*)application
  configurationForConnectingSceneSession:(UISceneSession*)connectingSceneSession
                                 options:(UISceneConnectionOptions*)options
{
  UISceneConfiguration* config =
    [[UISceneConfiguration alloc] initWithName:@"Default Configuration"
                                   sessionRole:connectingSceneSession.role];
  config.delegateClass = [AsepriteSceneDelegate class];
  return config;
}

@end

//////////////////////////////////////////////////////////////////////
// os::ios_* functions

namespace os {

void ios_process_events(double timeoutSecs)
{
  // Run the main run loop to dispatch UIKit events (touches, etc.)
  // until one source is handled or the timeout is reached.
  if (timeoutSecs < 0.0)
    timeoutSecs = 60.0 * 60.0;
  CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeoutSecs, true);

  // Process all the other pending sources without waiting
  while (CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true) == kCFRunLoopRunHandledSource) {
  }

  // Files to open
  std::vector<std::string> files;
  if (WindowIOS::instance()) {
    files = ios_take_pending_files();
    if (!files.empty()) {
      Event ev;
      ev.setType(Event::DropFiles);
      ev.setFiles(base::paths(files.begin(), files.end()));
      WindowIOS::instance()->queueEvent(ev);
    }
  }
}

void ios_wakeup()
{
  CFRunLoopWakeUp(CFRunLoopGetMain());
}

void* ios_main_view()
{
  return (__bridge void*)g_viewController.asepriteView;
}

float ios_content_scale()
{
  if (g_viewController.asepriteView)
    return g_viewController.asepriteView.contentScaleFactor;
  return [UIScreen mainScreen].scale;
}

gfx::Size ios_view_pixel_size()
{
  UIView* view = g_viewController.asepriteView;
  if (!view)
    return gfx::Size(0, 0);
  const CGFloat s = view.contentScaleFactor;
  return gfx::Size(int(std::round(view.bounds.size.width * s)),
                   int(std::round(view.bounds.size.height * s)));
}

void ios_haptic_feedback()
{
  UIImpactFeedbackGenerator* generator =
    [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
  [generator impactOccurred];
}

bool ios_open_url(const std::string& url)
{
  NSURL* nsurl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
  if (!nsurl || !nsurl.scheme)
    return false;
  [[UIApplication sharedApplication] openURL:nsurl options:@{} completionHandler:nil];
  return true;
}

void ios_add_pending_file(const std::string& fn)
{
  std::lock_guard lock(g_pendingFilesMutex);
  g_pendingFiles.push_back(fn);
}

std::vector<std::string> ios_take_pending_files()
{
  std::lock_guard lock(g_pendingFilesMutex);
  std::vector<std::string> files;
  files.swap(g_pendingFiles);
  return files;
}

// os/mobile.h impl

void show_soft_keyboard(bool state)
{
  [g_viewController showSoftKeyboard:state];
}

int default_screen_scale()
{
  int scale = std::clamp(int(std::round(ios_content_scale())), 1, 4);
  const gfx::Size size = ios_view_pixel_size();
  const int minSide = std::min(size.w, size.h);
  // Keep at least 600 pixels of UI space in the smallest side
  while (scale > 1 && minSide > 0 && minSide / scale < 600)
    --scale;
  return scale;
}

bool set_clipboard_text(const std::string& text)
{
  [UIPasteboard generalPasteboard].string = [NSString stringWithUTF8String:text.c_str()];
  return true;
}

bool get_clipboard_text(std::string& text)
{
  NSString* str = [UIPasteboard generalPasteboard].string;
  if (!str)
    return false;
  text = [str UTF8String];
  return true;
}

} // namespace os

//////////////////////////////////////////////////////////////////////
// Entry point

int main(int argc, char* argv[])
{
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil, NSStringFromClass([AsepriteAppDelegate class]));
  }
}
