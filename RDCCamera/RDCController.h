/*
    File: RDCController.h
   Abstract: Implements a camera application using the ImageCaptureCore framework.
   Version: 1.0.00

   Copyright (C) 2013 Leon. All Rights Reserved.

 */

#import <Cocoa/Cocoa.h>
#import <ImageCaptureCore/ImageCaptureCore.h>

@interface RDCController : NSObject<ICDeviceBrowserDelegate, ICCameraDeviceDelegate, ICCameraDeviceDownloadDelegate>
{
    ICDeviceBrowser*              mDeviceBrowser;
    NSMutableArray*               mCameras;

    IBOutlet  NSTableView*        mCamerasTableView;
    IBOutlet  NSArrayController*  mCamerasController;
    IBOutlet  NSTableView*        mCameraContentTableView;
    IBOutlet  NSArrayController*  mMediaFilesController;
}

@property(retain)   NSMutableArray* cameras;
@property(readonly) BOOL canDownload;

@end
