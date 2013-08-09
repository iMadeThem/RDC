/**
 * RDCCamera.cpp -- RDCCamera class realization.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#include "RDCCamera.h"
#include "MacICACamera.h"
#include <iostream>
#include <string>
#include <sys/time.h>
/* Header providing parametric manipulators */
#include <iomanip>
#include <cassert>

using namespace std;

RDCCamera::Notification* RDCCamera::cameraAddedNotification_ = 0;

/**
 * Constructor.
 */
RDCCamera::RDCCamera()
{
    imageAddedNotification_ = 0;
    imageDeletedNotification_ = 0;
    captureCompletedNotification_ = 0;
    cameraRemovedNotification_ = 0;
}

RDCCamera::~RDCCamera()
{}

/**
 * Debug log output file stream.
 */
std::ofstream RDCCamera::debugLog;

/**
 * Scan the system to get available cameras.
 */
void RDCCamera::cameraInventory(void)
{
    MacICACamera::cameraInventory();
}

string RDCCamera::maker(void) const
{
    return "UNKNOWN";
}

string RDCCamera::model(void) const
{
    return "UNKNOWN";
}

int RDCCamera::openSession(void)
{
    return 0;
}

int RDCCamera::closeSession(void)
{
    return 0;
}

void RDCCamera::heartbeat(void)
{}

int RDCCamera::batteryLevel(void)
{
    return -1;
}

// Image size.
int RDCCamera::getImageSizeIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setImageSizeIndex(int)
{}

bool RDCCamera::setImageSizeSucceed(void)
{
    return false;
}

// Exposure program.
int RDCCamera::getExposureProgramIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setExposureProgramIndex(int)
{}

bool RDCCamera::setExposureProgramSucceed(void)
{
    return false;
}

// Exposure time, shutter speed.
int RDCCamera::getExposureTimeIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setExposureTimeIndex(int)
{}

bool RDCCamera::setExposureTimeSucceed(void)
{
    return false;
}

// f-number, aperture value.
int RDCCamera::getFNumberIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setFNumberIndex(int)
{}

bool RDCCamera::setFNumberSucceed(void)
{
    return false;
}

// ISO value.
int RDCCamera::getISOIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setISOIndex(int)
{}

bool RDCCamera::setISOSucceed(void)
{
    return false;
}

// Flash mode.
int RDCCamera::getFlashModeIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setFlashModeIndex(int)
{}

bool RDCCamera::setFlashModeSucceed(void)
{
    return false;
}

// Focus mode.
int RDCCamera::getFocusModeIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setFocusModeIndex(int)
{}

bool RDCCamera::setFocusModeSucceed(void)
{
    return false;
}

// White balance mode.
int RDCCamera::getWhiteBalanceIndex(std::vector<string>& values)
{
    values.clear();
    return -1;
}

void RDCCamera::setWhiteBalanceIndex(int)
{}

bool RDCCamera::setWhiteBalanceSucceed(void)
{
    return false;
}

CaptureResponse_t RDCCamera::captureVolatileImage(void)
{
    return captureImage();
}

const list<FileKey_t>& RDCCamera::imageList(void)
{
    markImageNotification();
    return imageList_;
}

void RDCCamera::getImageByteData(long key, BYTE* data, bool)
{
    data = 0;
    cerr << "RDCCamera::getImageByteData: Not implemented!" << endl;
}

void RDCCamera::getImageThumbnail(long key, char*& buf, size_t& buf_len)
{
    buf = 0;
    buf_len = 0;
    cerr << "RDCCamera::getImageThumbnail: Not implemented!" << endl;
}

RDCCamera::Notification::Notification()
{}

RDCCamera::Notification::~Notification()
{}

RDCCamera::Notification::onImageAdded(RDCCamera*, const RDCCamera::FileKey_t &)
{
    debugLog << "RDCCamera: unimplemented onImageAdded notification." << endl << flush;
}

RDCCamera::Notification::onImageDeleted(RDCCamera*, const RDCCamera::FileKey_t &)
{
    debugLog << "RDCCamera: unimplemented onImageDeleted notification." << endl << flush;
}

RDCCamera::Notification::onCaptureCompleted(RDCCamera*)
{
    debugLog << "RDCCamera: unimplemented onCaptureCompleted notification." << endl << flush;
}

RDCCamera::Notification::onCameraAttached(RDCCamera*)
{
    debugLog << "RDCCamera: unimplemented onCameraAttached notification." << endl << flush;
}

RDCCamera::Notification::onCameraDetached(RDCCamera*)
{
    debugLog << "RDCCamera: unimplemented onCameraDetached notification." << endl << flush;
}

void RDCCamera::registerImageAddedNotification(RDCCamera::Notification* notification)
{
    assert(0 == notificationImageAdded_ || 0 == notification);
    notificationImageAdded_ = notification;
}

void RDCCamera::registerImageDeletedNotification(RDCCamera::Notification* notification)
{
    assert(0 == notificationImageDeleted_ || 0 == notification);
    notificationImageDeleted_ = notification;
}

void RDCCamera::markImageNotification(void)
{
    debugLog << TIMESTAMP << ": RDCCamera::markImageNotification ..." << endl << flush;

    // get the present image list from derived class.
    list<FileKey_t> newImageList;
    scanImages(newImageList); // implemented by derived class.

    // get a map of existing items, used for detecting added/delete images.
    map<FileKey_t, int> flagMap;
    for(list<FileKey_t>::iterator curr = imageList_.begin(); curr != imageList_.end(); curr++) {
        flagMap[*curr] = -1;
    }

    for(list<FileKey_t>::iterator curr = newImageList.begin(); curr != newImageList.end(); curr++) {
        flagMap[*curr] += 1;
    }

    // update the list with new one
    imageList_ = newImageList;

    // mark the status of images.
    // status <  0: IMAGE IS DELETED;
    // status == 0: IMAGE EXISTED BEFORE AND NOW;
    // status >  0: IMAGE IS NEWLY ADDED;
    for(map<FileKey_t, int>::const_iterator curr = flagMap.begin(); curr != flagMap.end(); curr++) {
        if(0 == curr->second) {
            continue;
        } else if(0 < curr->second) {
            markImageAdded_(curr->first);
        } else {
            markImageDeleted_(curr->first);
        }
    }
} // ~markImageNotification

void RDCCamera::markImagedAdded_(const FileKey_t& file)
{
    debugLog << TIMESTAMP << ": Camera added new image: " << file.second <<
    ", (id = " << file.first << ")" << endl << flush;
    if(notificationImageAdded_) {
        // call notification listener.
        imageAddedNotification_->onImageAdded(this, file);
    }
}

void RDCCamera::markImageDeleted_(const FileKey_t& file)
{
    debugLog << TIMESTAMP << ": Camera deleted image: " << file.second <<
    ", (id = " << file.first << ")" << endl << flush;
    if(notificationImageDeleted_) {
        // call notification listener.
        imageDeletedNotification_->onImageDeleted(this, file);
    }
}

void RDCCamera::registerCaptureCompletedNotification(RDCCamera::Notification* notification)
{
    assert(0 == notificationCaptureCompleted_ || 0 == notification);
    notificationCaptureCompleted_ = notification;
}

void RDCCamera::markCaptureCompleted(void)
{
    debugLog << TIMESTAMP << ": RDCCamera::markCaptureCompleted ..." << endl << flush;
    if(notificationCaptureCompleted_) {
        notificationCaptureCompleted_->onCaptureCompleted(this);
    }
}

/* This method is static because it is used to nofify new camera, not the
 * exsiting ones.
 */
void RDCCamera::registerCameraAttachedNotification(RDCCamera::Notification* notification)
{
    assert(0 == notificationCameraAttached_ || 0 == notification);
    notificationCameraAttached_ = notification;
}

void RDCCamera::markCameraAttached(RDCCamera* camera)
{
    debugLog << TIMESTAMP << ": RDCCamera::markCameraAttached ..." << endl << flush;
    if(notificationCameraAttached_) {
        notificationCameraAttached_->onCameraAttached(camera);
    }
}

void RDCCamera::registerCameraDetachedNotification(RDCCamera::Notification* notification)
{
    assert(0 == notificationCameraDetached_ || 0 == notification);
    notificationCameraDetached_ = notification;
}

void RDCCamera::markCameraDetached(RDCCamera* camera)
{
    debugLog << TIMESTAMP << ": RDCCamera::markCameraDetached ..." << endl << flush;
    if(notificationCameraDetached_) {
        notificationCameraDetached_->onCameraDetached(camera);
    }
}

void debugDump(std::ostream& out, const std::string& detail) const
{
    out << "RDCCamera::debugDump[" << detail << "]" << endl;
}

std::ostream TIMESTAMP(std::ostream& out)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    long secs = tv.tv_sec;
    unsigned long usecs = tv.tv_usec;
    while(usecs = > 1000000) {
        secs += 1;
        usecs -= 1000000;
    }
    out << secs << "." << setw(6) << usecs;
    return out;
}
