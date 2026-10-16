# Aseprite for Android

This directory contains the Android project (Gradle) to compile
Aseprite as an Android app (`.apk`). The native code is the same
Aseprite C++ code compiled with the Android NDK and a new Android
backend for [laf](../laf) (`laf/os/android`).

## Features

* Full Aseprite (same UI, all tools, scripting, extensions, etc.)
  running full-screen in landscape mode.
* The screen scale is selected automatically in the first run
  depending on the screen density (you can change it later in
  *Edit > Preferences > General > Screen Scaling/UI Elements Scaling*).
* **Touch screen** support:
  * **Tap**: left click.
  * **Drag with one finger**: left button drag (draw, select, move, etc.).
  * **Long press** (~0.5 seconds without moving, you'll feel a
    vibration) and release: right click (context menus, draw with
    the background color, etc.).
  * **Two fingers**: pan (scroll) the canvas and lists.
  * **Pinch with two fingers**: zoom in/out.
  * **Two-finger tap**: Undo.
  * **Three-finger tap**: Redo.
* **Stylus** (S Pen, USI pens, etc.): pressure sensitivity, hover
  cursor, the side button works as the right mouse button, and the
  eraser end is detected.
* **Mouse and keyboard** (Bluetooth/USB, Chromebooks, Samsung DeX):
  all buttons, mouse wheel and keyboard shortcuts work as on desktop.
* **Touch bar** (vertical bar at the right side) with big buttons:
  *Undo*, *Redo*, *R-Click* (the next touch will be a right click),
  *Shift*, *Ctrl*, *Alt*, *Space* (sticky modifiers to access tools
  like lines with Shift, the move tool with Ctrl, the eyedropper with
  Alt, or scrolling with Space), *Fit* (fit the sprite on screen) and
  *Keys* (show/hide the on-screen keyboard to use keyboard shortcuts).
* On-screen keyboard is displayed automatically when a text field is
  focused.
* The Android *Back* button works as the *Esc* key.
* Text copy/paste with the Android clipboard.
* Open images/sprites from other apps (*Open with...*/*Share*).
* Access to the device storage (Pictures, Download, Documents, etc.)
  through the Aseprite file selector after granting the *All files
  access* permission (requested in the first run). Without this
  permission you can use the app-specific folder
  `Android/data/org.aseprite.android/files`.
* Modified sprites are backed up automatically when the app goes to
  background (Android can close apps in background at any moment),
  and they can be restored from *File > Recover Files* / the Home tab.

## Requirements

* [Android Studio](https://developer.android.com/studio) (or the
  Android command-line tools) with:
  * Android SDK Platform 36
  * NDK `28.2.13676358`
  * CMake `3.31.6`
* Python 3 and `ninja` (`pip install ninja`) to compile Skia.
* About 15 GB of free disk space.

## 1. Compile Skia for Android

Aseprite uses [Skia](https://github.com/aseprite/skia) (m124) to
render the UI. There are no pre-built Skia packages for Android, so
you have to compile it (it takes ~10-20 minutes):

    cd android
    ./build-skia.sh C:/deps/skia $LOCALAPPDATA/Android/Sdk/ndk/28.2.13676358

On Windows you can run this script from Git Bash. If `python3` is not
available (only `python`), create a copy of `python.exe` named
`python3.exe` in a directory of your `PATH`.

The libraries will be generated in `C:/deps/skia/out/Android-arm64`
(for phones/tablets) and `C:/deps/skia/out/Android-x64` (for the
emulator and Chromebooks).

## 2. Compile the `gen` tool for your computer

Aseprite generates some C++ code from XML files during the build
using a small tool called `gen`. As this tool must run on your
computer (not on Android), it has to be compiled separately:

* Windows (requires Visual Studio 2022 with the C++ workload):

      android\build-gen.cmd C:\deps\build-gen-host

* Linux/macOS:

      ./android/build-gen.sh ~/deps/build-gen-host

## 3. Compile the APK

Change the `aseprite.skiaDir` and `aseprite.genExe` properties in
[gradle.properties](gradle.properties) if you compiled Skia/gen in
other directories, and create a `local.properties` file with the path
to the Android SDK:

    sdk.dir=C\:/Users/YOUR_USER/AppData/Local/Android/Sdk

Then:

    cd android
    ./gradlew assembleRelease

The APK will be in `app/build/outputs/apk/release/app-release.apk`.
It's signed with the debug key so you can install it directly:

    adb install -r app/build/outputs/apk/release/app-release.apk

You can also open the `android` directory with Android Studio.

To compile only for phones (faster), use:

    ./gradlew assembleRelease -Paseprite.abis=arm64-v8a

## Implementation details

* `laf/os/android/`: Android backend for laf-os (`NativeActivity` +
  `android_native_app_glue`). Input events (touch, stylus, mouse,
  keyboard) are converted to `os::Event`s, and the Skia raster surface
  is blitted to the `ANativeWindow`.
* `laf/os/skia/skia_window_android.cpp`: Skia window for Android.
* `src/app/ui/touch_bar.cpp`: on-screen bar with big buttons.
* `android/app/src/main/java/.../AsepriteActivity.java`: Java part
  of the activity (data files extraction, storage permission,
  on-screen keyboard, clipboard, intents to open files).
* Aseprite data files (`data/`) are packed in `assets/data.zip` and
  extracted to the app internal storage in the first run (or after an
  update).
* User configuration (`aseprite.ini`, extensions, etc.) is saved in
  the app internal storage.

Please note that Aseprite is distributed under the terms of the
[EULA](../EULA.txt): you can compile it for your personal use, but
you cannot redistribute the compiled app.
