/*
 *   File: RDCController.m
 * Abstract: Implements a camera application using the ImageCaptureCore framework.
 * Version: 1.0.00
 *
 * Copyright (C) 2013 Leon. All Rights Reserved.
 *
 */

#import "RDCController.h"

@interface NSImageFromCGImageRef : NSValueTransformer {}
@end

@implementation NSImageFromCGImageRef
+ (Class) transformedValueClass      {
    return [NSImage class];
}
+ (BOOL) allowsReverseTransformation {
    return NO;
}
- (id) transformedValue:(id)item {
    if ( item )
        return [[NSImage alloc] initWithCGImage:(CGImageRef)item size:NSZeroSize];
    else
        return nil;
}
@end


@implementation RDCController

@synthesize cameras = mCameras;

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

#pragma mark -
#pragma mark Initialization & Cleanup methods
- (void) applicationDidFinishLaunching:(NSNotification *)notification {
    mCameras = [[NSMutableArray alloc] initWithCapacity:0];

    // Get an instance of ICDeviceBrowser
    mDeviceBrowser = [[ICDeviceBrowser alloc] init];
    // Assign a delegate
    mDeviceBrowser.delegate = self;
    // Look for cameras in all available locations
    mDeviceBrowser.browsedDeviceTypeMask = mDeviceBrowser.browsedDeviceTypeMask | ICDeviceTypeMaskCamera
        | ICDeviceLocationTypeMaskLocal
        | ICDeviceLocationTypeMaskShared
        | ICDeviceLocationTypeMaskBonjour
        | ICDeviceLocationTypeMaskBluetooth
        | ICDeviceLocationTypeMaskRemote;
    // Start browsing for cameras
    [mDeviceBrowser start];

}


- (void) applicationWillTerminate:(NSNotification *)notification {
    mDeviceBrowser.delegate = NULL;

    [mDeviceBrowser stop];
}

#pragma mark -
#pragma mark Getter methods

/* The canDownload property reports whether or not there are any images available for download.
 */
- (BOOL) canDownload {
    if ( [[mMediaFilesController selectedObjects] count] )
        return YES;
    else
        return NO;
}

#pragma mark -
#pragma mark Methods to download media files from the device

/* The downloadFiles method is called when the Download button is pressed to download all available
 * media files from the device. Once a file is successfully downloaded, the didDownloadFile method is also called.
 */
- (void) downloadFiles:(NSArray *)files {
    NSDictionary * options = [NSDictionary dictionaryWithObject:[NSURL fileURLWithPath:[@"~/Pictures" stringByExpandingTildeInPath]] forKey:ICDownloadsDirectoryURL];

    for ( ICCameraFile * f in files ) {
        [f.device requestDownloadFile:f options:options downloadDelegate:self didDownloadSelector:@selector(didDownloadFile:error:options:contextInfo:) contextInfo:NULL];
    }
}

/* Once a file is successfully downloaded, the didDownloadFile method is called from the downloadFiles method.
 */
- (void) didDownloadFile:(ICCameraFile *)file error:(NSError *)error options:(NSDictionary *)options contextInfo:(void *)contextInfo {
    NSLog(@"didDownloadFile called with:\n");
    NSLog(@"  file:        %@\n", file);
    NSLog(@"  error:       %@\n", error);
    NSLog(@"  options:     %@\n", options);
    NSLog(@"  contextInfo: %p\n", contextInfo);
}

#pragma mark -
#pragma mark ICDeviceBrowser delegate methods
/* This message is sent to the delegate when a device has been added. This code adds the device to the cameras array.
 */
- (void) deviceBrowser:(ICDeviceBrowser *)browser didAddDevice:(ICDevice *)addedDevice moreComing:(BOOL)moreComing {
    NSLog(@"deviceBrowser:didAddDevice:moreComing: \n%@\n", addedDevice);

    if ( addedDevice.type & ICDeviceTypeCamera ) {
        addedDevice.delegate = self;

        // This triggers KVO messages to AppController
        // to add the new camera object to the cameras array.
        [[self mutableArrayValueForKey:@"cameras"] addObject:addedDevice];
    }
}


/* The required delegate method didRemoveDevice will handle the removal of camera devices. This message is sent to
 * the delegate to inform that a device has been removed.
 */
- (void) deviceBrowser:(ICDeviceBrowser *)browser didRemoveDevice:(ICDevice *)device moreGoing:(BOOL)moreGoing;
{
    NSLog(@"deviceBrowser:didRemoveDevice:moreGoing: \n%@\n", device);

    device.delegate = NULL;

    // This triggers KVO messages to AppController
    // to remove the camera object from the cameras array.
    [[self mutableArrayValueForKey:@"cameras"] removeObject:device];
}

#pragma mark -
#pragma mark ICDevice & ICCameraDevice delegate methods
/* A delegate of ICDevice must conform to ICDeviceDelegate protocol. The required delegate method didRemoveDevice
 * will handle the removal of camera devices. This message is sent to the delegate to inform that a device has been removed.
 */
- (void) didRemoveDevice:(ICDevice *)removedDevice {
    NSLog(@"didRemoveDevice: \n%@\n", removedDevice);
    [mCamerasController removeObject:removedDevice];
}



@end
