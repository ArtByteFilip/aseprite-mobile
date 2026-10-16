// Aseprite
// Copyright (C) 2026  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifndef APP_UI_TOUCH_BAR_H_INCLUDED
#define APP_UI_TOUCH_BAR_H_INCLUDED
#pragma once

#include "os/keys.h"
#include "ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace app {

// Vertical bar with big buttons for touch screens (Android): undo,
// redo, right-click mode, modifiers (Shift/Ctrl/Alt), on-screen
// keyboard, etc.
class TouchBar : public ui::Widget {
public:
  TouchBar();
  ~TouchBar();

  // Executes the action associated to a gesture recognized by the
  // platform (e.g. two-finger tap = undo).
  static void executeCommand(const char* commandId);

protected:
  bool onProcessMessage(ui::Message* msg) override;
  void onSizeHint(ui::SizeHintEvent& ev) override;
  void onPaint(ui::PaintEvent& ev) override;

private:
  struct Item {
    std::string text;
    bool toggle = false;
    std::function<void()> onClick;
    std::function<bool()> isChecked;
  };

  int itemHeight() const;
  int hitTest(const gfx::Point& screenPos) const;
  gfx::Rect itemBounds(int i) const;
  void toggleModifier(os::KeyModifiers modifier, os::KeyScancode scancode);

  std::vector<Item> m_items;
  int m_pressed = -1;
  int m_hot = -1;
  bool m_keyboardVisible = false;
};

} // namespace app

#endif
