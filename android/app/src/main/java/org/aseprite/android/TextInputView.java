// Aseprite
// Copyright (C) 2026  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

package org.aseprite.android;

import android.content.Context;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

// Invisible view that receives the text from the on-screen keyboard
// (IME) and sends it to the native code. Key events (Backspace,
// Enter, arrows, etc.) are delivered directly to the NativeActivity
// input queue.
class TextInputView extends View {

    TextInputView(Context context) {
        super(context);
        setFocusable(true);
        setFocusableInTouchMode(true);
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        outAttrs.inputType = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN | EditorInfo.IME_FLAG_NO_EXTRACT_UI
            | EditorInfo.IME_ACTION_DONE;
        return new AsepriteInputConnection(this);
    }

    private static class AsepriteInputConnection extends BaseInputConnection {
        AsepriteInputConnection(View view) {
            super(view, false);
        }

        @Override
        public boolean commitText(CharSequence text, int newCursorPosition) {
            if (text != null && text.length() > 0)
                AsepriteActivity.sendText(text.toString());
            return true;
        }

        @Override
        public boolean setComposingText(CharSequence text, int newCursorPosition) {
            // Ignore composing text (we don't have a preview of it),
            // the final text is received in commitText()
            return true;
        }

        @Override
        public boolean finishComposingText() {
            return true;
        }

        @Override
        public boolean deleteSurroundingText(int beforeLength, int afterLength) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < beforeLength; ++i)
                sb.append('\b');
            if (sb.length() > 0)
                AsepriteActivity.sendText(sb.toString());
            return true;
        }

        @Override
        public boolean performEditorAction(int editorAction) {
            AsepriteActivity.sendText("\n");
            return true;
        }

        @Override
        public boolean sendKeyEvent(KeyEvent event) {
            // Keys are delivered to the native input queue
            return super.sendKeyEvent(event);
        }
    }
}
