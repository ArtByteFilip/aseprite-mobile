// Aseprite
// Copyright (C) 2026  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif

#include "app/ui/touch_bar.h"

#include "app/commands/command.h"
#include "app/commands/commands.h"
#include "app/ui/skin/skin_theme.h"
#include "app/ui_context.h"
#include "os/event.h"
#include "os/event_queue.h"
#include "os/system.h"
#include "ui/display.h"
#include "ui/graphics.h"
#include "ui/message.h"
#include "ui/paint_event.h"
#include "ui/scale.h"
#include "ui/size_hint_event.h"

#if LAF_MOBILE
  #include "os/mobile.h"
#endif

#include <algorithm>

namespace app {

using namespace app::skin;
using namespace ui;

namespace {

os::KeyModifiers virtual_modifiers()
{
#if LAF_MOBILE
  return os::virtual_modifiers();
#else
  return os::kKeyNoneModifier;
#endif
}

bool right_click_mode()
{
#if LAF_MOBILE
  return os::right_click_mode();
#else
  return false;
#endif
}

} // anonymous namespace

TouchBar::TouchBar() : Widget(kGenericWidget)
{
  m_items.push_back({ "Undo", false, [] { executeCommand(CommandId::Undo()); }, nullptr });
  m_items.push_back({ "Redo", false, [] { executeCommand(CommandId::Redo()); }, nullptr });
  m_items.push_back(
    { "R-Click",
      true,
      [this] {
#if LAF_MOBILE
        os::set_right_click_mode(!os::right_click_mode());
#endif
        invalidate();
      },
      [] { return right_click_mode(); } });
  m_items.push_back({ "Shift",
                      true,
                      [this] { toggleModifier(os::kKeyShiftModifier, os::kKeyLShift); },
                      [] { return (virtual_modifiers() & os::kKeyShiftModifier) != 0; } });
#if __APPLE__
  // On Apple platforms the shortcuts use the Command key instead of Ctrl
  m_items.push_back({ "Cmd",
                      true,
                      [this] { toggleModifier(os::kKeyCmdModifier, os::kKeyCommand); },
                      [] { return (virtual_modifiers() & os::kKeyCmdModifier) != 0; } });
#else
  m_items.push_back({ "Ctrl",
                      true,
                      [this] { toggleModifier(os::kKeyCtrlModifier, os::kKeyLControl); },
                      [] { return (virtual_modifiers() & os::kKeyCtrlModifier) != 0; } });
#endif
  m_items.push_back({ "Alt",
                      true,
                      [this] { toggleModifier(os::kKeyAltModifier, os::kKeyAlt); },
                      [] { return (virtual_modifiers() & os::kKeyAltModifier) != 0; } });
  m_items.push_back({ "Space",
                      true,
                      [this] { toggleModifier(os::kKeySpaceModifier, os::kKeySpace); },
                      [] { return (virtual_modifiers() & os::kKeySpaceModifier) != 0; } });
  m_items.push_back({ "Fit", false, [] { executeCommand(CommandId::FitScreen()); }, nullptr });
  m_items.push_back({ "Keys",
                      true,
                      [this] {
                        m_keyboardVisible = !m_keyboardVisible;
#if LAF_MOBILE
                        os::show_soft_keyboard(m_keyboardVisible);
#endif
                        invalidate();
                      },
                      [this] { return m_keyboardVisible; } });

  initTheme();
}

TouchBar::~TouchBar()
{
#if LAF_MOBILE
  os::set_virtual_modifiers(os::kKeyNoneModifier);
  os::set_right_click_mode(false);
#endif
}

// static
void TouchBar::executeCommand(const char* commandId)
{
  Command* cmd = Commands::instance()->byId(commandId);
  if (cmd && UIContext::instance())
    UIContext::instance()->executeCommandFromMenuOrShortcut(cmd);
}

void TouchBar::toggleModifier(os::KeyModifiers modifier, os::KeyScancode scancode)
{
  const os::KeyModifiers mods = virtual_modifiers();
  const bool enable = ((mods & modifier) == 0);
  const os::KeyModifiers newMods = (enable ? os::KeyModifiers(mods | modifier) :
                                             os::KeyModifiers(mods & ~modifier));
#if LAF_MOBILE
  os::set_virtual_modifiers(newMods);
#endif

  // Generate the key event so the active editor state can be
  // updated (e.g. to change the mouse cursor or the tool preview).
  os::Event ev;
  ev.setType(enable ? os::Event::KeyDown : os::Event::KeyUp);
  ev.setScancode(scancode);
  ev.setModifiers(newMods);
  ev.setUnicodeChar(0);
  ev.setRepeat(0);
  if (Display* display = this->display())
    ev.setWindow(base::AddRef(display->nativeWindow()));
  os::queue_event(ev);

  invalidate();
}

int TouchBar::itemHeight() const
{
  return std::max(int(font()->lineHeight()) * 2 + 6 * guiscale(), 24 * guiscale());
}

gfx::Rect TouchBar::itemBounds(int i) const
{
  const gfx::Rect rc = clientBounds();
  const int h = itemHeight();
  const int gap = 2 * guiscale();
  return gfx::Rect(rc.x + gap, rc.y + gap + i * (h + gap), rc.w - 2 * gap, h);
}

int TouchBar::hitTest(const gfx::Point& screenPos) const
{
  const gfx::Point pos = screenPos - bounds().origin();
  for (int i = 0; i < int(m_items.size()); ++i) {
    if (itemBounds(i).contains(pos))
      return i;
  }
  return -1;
}

bool TouchBar::onProcessMessage(Message* msg)
{
  switch (msg->type()) {
    case kMouseDownMessage: {
      auto* mouseMsg = static_cast<MouseMessage*>(msg);
      m_pressed = hitTest(mouseMsg->position());
      if (m_pressed >= 0) {
        captureMouse();
        invalidate();
      }
      return true;
    }

    case kMouseMoveMessage: {
      auto* mouseMsg = static_cast<MouseMessage*>(msg);
      const int hot = hitTest(mouseMsg->position());
      if (hot != m_hot) {
        m_hot = hot;
        invalidate();
      }
      return true;
    }

    case kMouseUpMessage: {
      auto* mouseMsg = static_cast<MouseMessage*>(msg);
      const int pressed = m_pressed;
      m_pressed = -1;
      if (hasCapture())
        releaseMouse();
      invalidate();

      if (pressed >= 0 && pressed == hitTest(mouseMsg->position())) {
        // Copy the action as it could modify the widget
        auto onClick = m_items[pressed].onClick;
        if (onClick)
          onClick();
      }
      return true;
    }

    case kMouseLeaveMessage:
      if (m_hot >= 0) {
        m_hot = -1;
        invalidate();
      }
      break;
  }
  return Widget::onProcessMessage(msg);
}

void TouchBar::onSizeHint(SizeHintEvent& ev)
{
  int w = 0;
  for (const auto& item : m_items)
    w = std::max(w, int(font()->textLength(item.text)));
  w += 12 * guiscale();
  w = std::max(w, itemHeight());

  const int gap = 2 * guiscale();
  const int h = gap + int(m_items.size()) * (itemHeight() + gap);
  ev.setSizeHint(gfx::Size(w, h));
}

void TouchBar::onPaint(PaintEvent& ev)
{
  auto* theme = SkinTheme::get(this);
  Graphics* g = ev.graphics();

  g->fillRect(theme->colors.workspace(), clientBounds());

  for (int i = 0; i < int(m_items.size()); ++i) {
    const Item& item = m_items[i];
    const gfx::Rect rc = itemBounds(i);
    const bool checked = (item.isChecked && item.isChecked());

    gfx::Color face = theme->colors.face();
    gfx::Color text = theme->colors.text();
    if (checked || i == m_pressed) {
      face = theme->colors.selected();
      text = theme->colors.selectedText();
    }
    else if (i == m_hot) {
      face = theme->colors.hotFace();
    }

    g->fillRect(face, rc);
    g->drawRect(theme->colors.separatorLabel(), rc);
    g->drawAlignedUIText(item.text, text, gfx::ColorNone, rc, CENTER | MIDDLE);
  }
}

} // namespace app
