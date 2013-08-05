//
//  RDAppDelegate.m
//  RemoteDLSR
//
//  Created by Leon on 13-7-13.
//  Copyright (c) 2013年 Leon. All rights reserved.
//

#import "RDAppDelegate.h"
#import <AppKit/NSImage.h>

@implementation RDAppDelegate

@synthesize imagePreview;

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
- (IBAction)onClickPickPhotoBtn:(id)sender {
    NSOpenPanel* panel = [NSOpenPanel openPanel];
    // This method displays the panel and returns immediately.
    // The completion handler is called when the user selects an
    // item or cancels the panel.
    NSArray *imageTypes = [NSImage imageTypes];
    [panel setAllowedFileTypes:imageTypes];
    
    [panel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSURL* imageUrl = [[panel URLs] objectAtIndex:0];
            // Open the document.
            NSLog(@"DOC: %@",imageUrl);
            
            NSData *imageData = [NSData dataWithContentsOfURL:imageUrl];
            [imagePreview setImage: [[NSImage init] initWithData: imageData]];
            
        }
        
        
    }];
    
}
@end
