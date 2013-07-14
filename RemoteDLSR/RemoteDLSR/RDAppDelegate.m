//
//  RDAppDelegate.m
//  RemoteDLSR
//
//  Created by Leon on 13-7-13.
//  Copyright (c) 2013年 Leon. All rights reserved.
//

#import "RDAppDelegate.h"

@implementation RDAppDelegate

- (void) applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (IBAction) onClickCaptureBtn:(id)sender {
    // TODO: detect camera connected and detectable!
    //    if (!BOOL_CameraSelected) {
    //        // show alert
    //    } else {
    //        // Do capture with settings
    //    }

    [[NSAlert alertWithMessageText:@"No Camera Selected" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"No camera dected or selected! \nConnect your camera to the Mac then go to Camera setting to select it."] runModal];
}

- (IBAction) onClickTetheredBtn:(id)sender {
    // TODO: detect camera connected and detectable!
    //    if (!BOOL_CameraSelected) {
    //        // show alert
    //    } else {
    //        // Do capture with settings
    //    }
    [[NSAlert alertWithMessageText:@"No Camera Selected" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"No camera dected or selected! \nConnect your camera to the Mac then go to Camera setting to select it."] runModal];
}

- (IBAction) onClickTimeLapseBtn:(id)sender {
    // TODO: detect camera connected and detectable!
    //    if (!BOOL_CameraSelected) {
    //        // show alert
    //    } else {
    //        // Do capture with settings
    //    }
    [[NSAlert alertWithMessageText:@"No Camera Selected" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"No camera dected or selected! \nConnect your camera to the Mac then go to Camera setting to select it."] runModal];
}
@end
