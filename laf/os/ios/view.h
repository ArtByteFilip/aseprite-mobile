// LAF OS Library
// Copyright (C) 2026  Igara Studio S.A.
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifndef OS_IOS_VIEW_H_INCLUDED
#define OS_IOS_VIEW_H_INCLUDED
#pragma once

#ifdef __OBJC__

  #include <UIKit/UIKit.h>

// Hidden view used to show the on-screen keyboard and receive text.
@interface AsepriteKeyInputView : UIView <UIKeyInput>
@end

// Main view where the Skia surface is displayed and that receives
// all the input events.
@interface AsepriteView : UIView
- (void)presentImage:(CGImageRef)image
          imageWidth:(int)width
         imageHeight:(int)height
         windowScale:(int)scale;
@end

@interface AsepriteViewController : UIViewController
@property (nonatomic, strong) AsepriteView* asepriteView;
@property (nonatomic, strong) AsepriteKeyInputView* keyInputView;
- (void)showSoftKeyboard:(BOOL)state;
@end

// Returns the main view controller (created by the scene delegate)
AsepriteViewController* ios_main_view_controller();

#endif

#endif
