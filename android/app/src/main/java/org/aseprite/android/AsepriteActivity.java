// Aseprite
// Copyright (C) 2026  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

package org.aseprite.android;

import android.Manifest;
import android.app.AlertDialog;
import android.app.NativeActivity;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;
import android.provider.Settings;
import android.system.Os;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.window.OnBackInvokedDispatcher;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class AsepriteActivity extends NativeActivity {
    private static final String TAG = "aseprite";
    private static final int REQUEST_STORAGE = 1;

    static {
        // Load the library before NativeActivity does it, so the JNI
        // methods (native*) can be found from Java.
        System.loadLibrary("aseprite");
    }

    // Implemented in laf/os/android/android_app.cpp
    private static native void nativeOnTextInput(String text);
    private static native void nativeOnOpenFile(String path);
    private static native void nativeOnBackPressed();

    private TextInputView mInputView;
    private ClipboardManager mClipboard;
    private boolean mKeyboardVisible = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        setupEnvironment();

        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        hideSystemBars();

        mClipboard = (ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);

        // Hidden view used to receive text from the on-screen keyboard
        mInputView = new TextInputView(this);
        addContentView(mInputView, new ViewGroup.LayoutParams(1, 1));

        // The back button/gesture is converted to the Esc key (on
        // Android 13+ the back key event is not delivered to the
        // native input queue when predictive back is enabled)
        if (Build.VERSION.SDK_INT >= 33) {
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                OnBackInvokedDispatcher.PRIORITY_DEFAULT,
                () -> nativeOnBackPressed());
        }

        requestStoragePermission();
        handleIntent(getIntent());
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        handleIntent(intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemBars();
        updateHomeDirectory();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus)
            hideSystemBars();
    }

    @Override
    @SuppressWarnings("deprecation")
    public void onBackPressed() {
        // Android < 13: the back key is converted to Esc in the native
        // code, this is called only if the native code didn't handle it.
        nativeOnBackPressed();
    }

    // ------------------------------------------------------------------
    // Environment & data files

    private void setupEnvironment() {
        File filesDir = getFilesDir();
        File appDir = new File(filesDir, "aseprite");
        File dataDir = new File(appDir, "data");

        extractDataIfNeeded(appDir, dataDir);

        try {
            // $BINDIR = files/aseprite, so data files are located in
            // files/aseprite/data (see ResourceFinder::includeDataDir())
            Os.setenv("LAF_APP_PATH", new File(appDir, "aseprite").getAbsolutePath(), true);
            // User configuration in files/aseprite (aseprite.ini,
            // extensions, palettes, etc.)
            Os.setenv("ASEPRITE_USER_FOLDER", appDir.getAbsolutePath(), true);
            Os.setenv("XDG_CONFIG_HOME", filesDir.getAbsolutePath(), true);
            Os.setenv("TMPDIR", getCacheDir().getAbsolutePath(), true);
        }
        catch (Exception ex) {
            Log.e(TAG, "Cannot set environment variables", ex);
        }

        updateHomeDirectory();
    }

    // $HOME is used as the default folder for the file selector
    private void updateHomeDirectory() {
        File home;
        if (hasStoragePermission()) {
            home = Environment.getExternalStorageDirectory();
        }
        else {
            home = getExternalFilesDir(null);
            if (home == null)
                home = getFilesDir();
        }
        try {
            Os.setenv("HOME", home.getAbsolutePath(), true);
        }
        catch (Exception ex) {
            Log.e(TAG, "Cannot set HOME", ex);
        }
    }

    private void extractDataIfNeeded(File appDir, File dataDir) {
        String version = "unknown";
        try {
            PackageInfo info = getPackageManager().getPackageInfo(getPackageName(), 0);
            version = info.versionCode + "-" + info.lastUpdateTime;
        }
        catch (PackageManager.NameNotFoundException ignored) {
        }

        File versionFile = new File(dataDir, ".android-version");
        if (versionFile.exists() && version.equals(readFile(versionFile)))
            return;

        Log.i(TAG, "Extracting data files to " + dataDir);
        deleteRecursive(dataDir);
        appDir.mkdirs();

        try (InputStream raw = getAssets().open("data.zip");
             ZipInputStream zip = new ZipInputStream(new BufferedInputStream(raw, 65536))) {
            byte[] buffer = new byte[65536];
            ZipEntry entry;
            String rootPath = appDir.getCanonicalPath() + File.separator;
            while ((entry = zip.getNextEntry()) != null) {
                File out = new File(appDir, entry.getName());
                // Avoid zip-slip
                if (!out.getCanonicalPath().startsWith(rootPath))
                    continue;
                if (entry.isDirectory()) {
                    out.mkdirs();
                    continue;
                }
                out.getParentFile().mkdirs();
                try (OutputStream os = new FileOutputStream(out)) {
                    int n;
                    while ((n = zip.read(buffer)) > 0)
                        os.write(buffer, 0, n);
                }
            }
            writeFile(versionFile, version);
        }
        catch (IOException ex) {
            Log.e(TAG, "Cannot extract data files", ex);
        }
    }

    private static void deleteRecursive(File file) {
        if (file.isDirectory()) {
            File[] children = file.listFiles();
            if (children != null) {
                for (File child : children)
                    deleteRecursive(child);
            }
        }
        file.delete();
    }

    private static String readFile(File file) {
        try (InputStream is = new java.io.FileInputStream(file)) {
            byte[] buf = new byte[(int)file.length()];
            int n = is.read(buf);
            return new String(buf, 0, Math.max(0, n), "UTF-8");
        }
        catch (IOException ex) {
            return "";
        }
    }

    private static void writeFile(File file, String content) {
        try (OutputStream os = new FileOutputStream(file)) {
            os.write(content.getBytes("UTF-8"));
        }
        catch (IOException ex) {
            Log.e(TAG, "Cannot write " + file, ex);
        }
    }

    // ------------------------------------------------------------------
    // Storage permission

    private boolean hasStoragePermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
            return Environment.isExternalStorageManager();
        return checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
            == PackageManager.PERMISSION_GRANTED;
    }

    private void requestStoragePermission() {
        if (hasStoragePermission())
            return;

        if (getPreferences(MODE_PRIVATE).getBoolean("storage_asked", false))
            return;
        getPreferences(MODE_PRIVATE).edit().putBoolean("storage_asked", true).apply();

        new AlertDialog.Builder(this)
            .setTitle(R.string.storage_title)
            .setMessage(R.string.storage_message)
            .setPositiveButton(R.string.storage_allow, (dialog, which) -> {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                    try {
                        Intent intent = new Intent(
                            Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION,
                            Uri.parse("package:" + getPackageName()));
                        startActivity(intent);
                    }
                    catch (Exception ex) {
                        startActivity(new Intent(
                            Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION));
                    }
                }
                else {
                    requestPermissions(new String[] {
                            Manifest.permission.READ_EXTERNAL_STORAGE,
                            Manifest.permission.WRITE_EXTERNAL_STORAGE },
                        REQUEST_STORAGE);
                }
            })
            .setNegativeButton(R.string.storage_deny, null)
            .show();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(requestCode, permissions, results);
        updateHomeDirectory();
    }

    // ------------------------------------------------------------------
    // Open files from other apps

    private void handleIntent(Intent intent) {
        if (intent == null)
            return;

        Uri uri = null;
        String action = intent.getAction();
        if (Intent.ACTION_VIEW.equals(action) || Intent.ACTION_EDIT.equals(action)) {
            uri = intent.getData();
        }
        else if (Intent.ACTION_SEND.equals(action)) {
            uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
        }
        if (uri == null)
            return;

        // Avoid processing the same intent again (e.g. activity recreated)
        intent.setAction(Intent.ACTION_MAIN);

        final Uri finalUri = uri;
        new Thread(() -> {
            String path = resolveUriToFile(finalUri);
            if (path != null)
                nativeOnOpenFile(path);
        }).start();
    }

    // Returns a real file path for the given Uri (if we have access to
    // the file directly), or copies the content to a temporary file.
    private String resolveUriToFile(Uri uri) {
        if ("file".equals(uri.getScheme()))
            return uri.getPath();

        // Documents from the primary external storage
        if (hasStoragePermission()) {
            try {
                if (DocumentsContract.isDocumentUri(this, uri) &&
                    "com.android.externalstorage.documents".equals(uri.getAuthority())) {
                    String docId = DocumentsContract.getDocumentId(uri);
                    String[] parts = docId.split(":", 2);
                    if (parts.length == 2 && "primary".equalsIgnoreCase(parts[0])) {
                        File file = new File(Environment.getExternalStorageDirectory(), parts[1]);
                        if (file.canRead())
                            return file.getAbsolutePath();
                    }
                }
            }
            catch (Exception ignored) {
            }
        }

        // Copy the content to the cache directory
        String name = "sprite";
        try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                int index = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (index >= 0 && cursor.getString(index) != null)
                    name = cursor.getString(index).replaceAll("[/\\\\]", "_");
            }
        }
        catch (Exception ignored) {
        }

        File dir = new File(getCacheDir(), "opened");
        dir.mkdirs();
        File out = new File(dir, name);
        try (InputStream is = getContentResolver().openInputStream(uri);
             OutputStream os = new FileOutputStream(out)) {
            if (is == null)
                return null;
            byte[] buffer = new byte[65536];
            int n;
            while ((n = is.read(buffer)) > 0)
                os.write(buffer, 0, n);
            return out.getAbsolutePath();
        }
        catch (IOException ex) {
            Log.e(TAG, "Cannot open " + uri, ex);
            return null;
        }
    }

    // ------------------------------------------------------------------
    // UI helpers

    @SuppressWarnings("deprecation")
    private void hideSystemBars() {
        View decor = getWindow().getDecorView();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
            WindowInsetsController controller = decor.getWindowInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        }
        else {
            decor.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        }
    }

    // ------------------------------------------------------------------
    // Methods called from the native code (JNI)

    @SuppressWarnings("unused")
    public void showSoftKeyboard(final boolean state) {
        runOnUiThread(() -> {
            InputMethodManager imm =
                (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm == null)
                return;
            if (state) {
                mInputView.setFocusableInTouchMode(true);
                mInputView.requestFocus();
                imm.showSoftInput(mInputView, InputMethodManager.SHOW_IMPLICIT);
            }
            else if (mKeyboardVisible) {
                imm.hideSoftInputFromWindow(mInputView.getWindowToken(), 0);
                mInputView.clearFocus();
                hideSystemBars();
            }
            mKeyboardVisible = state;
        });
    }

    @SuppressWarnings("unused")
    public int getUnicodeChar(int keyCode, int metaState) {
        return new KeyEvent(KeyEvent.ACTION_DOWN, keyCode).getUnicodeChar(metaState);
    }

    @SuppressWarnings("unused")
    public void hapticFeedback() {
        runOnUiThread(() -> getWindow().getDecorView().performHapticFeedback(
            HapticFeedbackConstants.LONG_PRESS));
    }

    @SuppressWarnings("unused")
    public boolean openUrl(String url) {
        try {
            Intent intent;
            if (url.startsWith("http://") || url.startsWith("https://") ||
                url.startsWith("mailto:")) {
                intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            }
            else {
                return false;
            }
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
            return true;
        }
        catch (Exception ex) {
            Log.e(TAG, "Cannot open " + url, ex);
            return false;
        }
    }

    @SuppressWarnings("unused")
    public boolean setClipboardText(String text) {
        try {
            mClipboard.setPrimaryClip(ClipData.newPlainText("Aseprite", text));
            return true;
        }
        catch (Exception ex) {
            return false;
        }
    }

    @SuppressWarnings("unused")
    public String getClipboardText() {
        try {
            ClipData clip = mClipboard.getPrimaryClip();
            if (clip != null && clip.getItemCount() > 0) {
                CharSequence text = clip.getItemAt(0).coerceToText(this);
                if (text != null)
                    return text.toString();
            }
        }
        catch (Exception ignored) {
        }
        return null;
    }

    // Called from TextInputView (UI thread)
    static void sendText(String text) {
        nativeOnTextInput(text);
    }
}
