//
//  RDAppDelegate.h
//  RemoteDLSR
//
//  Created by Leon on 13-7-13.
//  Copyright (c) 2013年 Leon. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface RDAppDelegate : NSObject<NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;

// capture buttons
@property (assign) IBOutlet NSButton *btnCapture;
@property (assign) IBOutlet NSButton *btnTethered;

// pick photo to show in preview area
@property (assign) IBOutlet NSButton *btnPickPhoto;
// photo preview mode: 1:1 zoom or full size
@property (assign) IBOutlet NSMatrix *optZoomPhoto;

// label that display selected camera
@property (assign) IBOutlet NSTextField *labelDetectedCamara;

// camara settings tab view
@property (assign) IBOutlet NSTabView *tabSettings;
// Camera tab
// Settings tab
@property (assign) IBOutlet NSWindow *exposureMode;
@property (assign) IBOutlet NSWindow *aperture;
@property (assign) IBOutlet NSWindow *shutterSpped;
@property (assign) IBOutlet NSWindow *whiteBalance;
@property (assign) IBOutlet NSWindow *ISOValue;
@property (assign) IBOutlet NSWindow *focusMode;
@property (assign) IBOutlet NSWindow *flashMode;
// Timer tab
// Others tab
@property (assign) IBOutlet NSWindow *imageQuality;
@property (assign) IBOutlet NSWindow *imageSize;
// Photos tab

// capture actions
- (IBAction) onClickCaptureBtn:(id) sender;
- (IBAction) onClickTetheredBtn:(id) sender;
- (IBAction) onClickTimeLapseBtn:(id) sender;

// Camera tab
// Settings tab
// Timer tab
// Others tab
// Photos tab

@end
