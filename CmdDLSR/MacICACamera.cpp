/**
 * MacICACamera.cpp -- MacICACamera realization.
 * Author: Leon (leon(at)gmail.com)
 * Date  : 2013-08-09 [Friday, 221]
 */

#include "MacICACamera.h"
#include <iostream>

using namespace std;

/**
 * Create a MacICACamera instance from an ICAObject. That object should already be known to be a
 * camera. (The inventory method assure me of that.) Get the vendor and product, and use that to
 * generate the make and model strings.
 */
MacICACamera::MacICACamera(ICAObject dev)
{
    dev_ = dev;
    deviceDict_ = 0;
    refreshDeviceDictionary_();

    deviceModel_ = usbIdToName(getUsbIdFromDict_(deviceDict_));
}

MacICACamera::~MacICACamera()
{
    CFRelease(deviceDict_);
}

/*
 * Sometimes we have to reread the property dictionary from the
 * device.
 */
void MacICACamera::refreshDeviceDictionary_(void)
{
    if(deviceDict_ != 0) CFRelease(deviceDict_);

    ICACopyObjectPropertyDictionaryPB dev_dict_pb;
    memset(&dev_dict_pb, 0, sizeof dev_dict_pb);
    dev_dict_pb.object = dev_;
    dev_dict_pb.theDict = &deviceDict_;
    ICACopyObjectPropertyDictionary(&dev_dict_pb, 0);
}

string MacICACamera::controlClass(void) const
{
    return "Image Capture Architecture";
}

string MacICACamera::maker(void) const
{
    return deviceModel_.first;
}

string MacICACamera::model(void) const
{
    return deviceModel_.second;
}

int MacICACamera::openSession(void)
{
    ICAOpenSessionPB pb;
    memset(&pb, 0, sizeof pb);
    pb.deviceObject = dev_;
    ICAOpenSession(&pb, 0);
    session_id_ = pb.sessionID;

    notification_camera_ = this;
    return 0;
}

int MacICACamera::closeSession(void)
{
    notification_camera_ = 0;

    ICACloseSessionPB pb;
    memset(&pb, 0, sizeof pb);
    pb.sessionID = session_id_;
    ICACloseSession(&pb, 0);
    return 0;
}

CameraControl::CaptureResponse_t MacICACamera::captureImage(void)
{
    ICAObjectSendMessagePB send_pb;
    memset(&send_pb, 0, sizeof send_pb);
    send_pb.object = dev_;
    send_pb.message.messageType = kICAMessageCameraCaptureNewImage;
    send_pb.message.dataPtr = 0;
    send_pb.message.dataSize = 0;
    send_pb.message.dataType = 0;
    ICAObjectSendMessage(&send_pb, 0);

    return CAP_OK;
}

/*
 * Scan the images on the camera by refreshing the device dictionary and getting the "data" array
 * from the device. This is an array of images that I format into the file list for the caller.
 */

void MacICACamera::getImageByteData(long key, BYTE* data, bool delete_image)
{
    ICAObject image = icaImageObjectFromDevice_(key);
    if(image == 0) {
        data.clear();
        return;
    }

    // Given the image object, we first get the image details by getting the kICAPropertyImageData
    // property. That will have the size of the image data, which I can in turn use to retrieve the
    // full image via the ICACopyObjectData function.
    ICAGetPropertyByTypePB image_data_pb;
    memset(&image_data_pb, 0, sizeof image_data_pb);
    image_data_pb.object = image;
    image_data_pb.propertyType = kICAPropertyImageData;
    ICAGetPropertyByType(&image_data_pb, 0);
    size_t bufLen = image_data_pb.propertyInfo.dataSize;

    // Now get the image data directly from the image object.
    CFDataRef dataRef;
    ICACopyObjectDataPB data_pb;
    memset(&data_pb, 0, sizeof data_pb);
    data_pb.object = image;
    data_pb.startByte = 0;
    data_pb.requestedSize = bufLen;
    data_pb.data = &dataRef;
    ICAError rc = ICACopyObjectData(&data_pb, 0);
    assert(rc == 0);

    // Put the image data into the QByteArray that the caller passed in.
    data.resize(CFDataGetLength(dataRef));
    CFDataGetBytes(dataRef, CFRangeMake(0, bufLen), (UInt8 *) data.data());
    CFRelease(dataRef);

    if(delete_image) {
        ICAObjectSendMessagePB send_pb;
        memset(&send_pb, 0, sizeof send_pb);
        send_pb.object = image;
        send_pb.message.messageType = kICAMessageCameraDeleteOne;
        send_pb.message.dataPtr = 0;
        send_pb.message.dataSize = 0;
        send_pb.message.dataType = 0;
        ICAObjectSendMessage(&send_pb, 0);
    }
}

void MacICACamera::getImageThumbnail(long key, char*& buf, size_t& bufLen)
{
    ICAObject image = ica_image_object_from_dev_(key);
    CFDataRef data;

    ICACopyObjectThumbnailPB pb;
    memset(&pb, 0, sizeof pb);
    pb.object = image;
    pb.thumbnailFormat = kICAThumbnailFormatPNG;
    pb.thumbnailData = &data;
    ICACopyObjectThumbnail(&pb, 0);

    bufLen = CFDataGetLength(data);
    buf = new char[bufLen];
    CFDataGetBytes(data, CFRangeMake(0, bufLen), (UInt8 *) buf);
    CFRelease(data);
}
