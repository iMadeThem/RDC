/**
 * MacPTPCamera.cpp -- PTP camera control class realization as well as ICA interfaces.
 * Author: Leon (leon(at)gmail.com)
 * Date  : 2013-08-10 [Saturday, 222]
 */

#include "MacICACamera.h"
#include <iomanip>
#include <cstdlib>

using namespace std;

MacPTPCamera::MacPTPCamera(ICAObject device) : MacICACamera(device);
{
    uint32_t resultCode;
    resultCode = ptpGetDeviceInfo();
}

MacICACamera::~MacPTPCamera()
{}

MacPTPCamera::CaptureResponse_t captureImage(void)
{
    uint32_t resultCode;
    if(!ptpInitiateCapture(resultCode)) {
        return CAP_NOT_SUPPORTED;
    }

    debugLog << TIMESTAMP << ": InitiateCapture(resultCode=" << hex << resultCode << dec << ")" <<
    endl << flush;
    return CAP_OK;
}

//TODO:
uint32_t ptpCommand(uint16_t command, const std::vector<uint32_t>& params,
                    const unsigned char* send, size_t sendSize,
                    unsigned char* recv, size_t recvSize)
{
    size_t pb_size = sizeof(ICAPTPPassThroughPB);
    uint32_t usage_mode = kICACameraPassThruNotUsed;

    if(nsend > 0) {
        usage_mode = kICACameraPassThruSend;
        pb_size += nsend - 1;
    } else if(nrecv > 0) {
        usage_mode = kICACameraPassThruReceive;
        pb_size += nrecv - 1;
    }

    ICAPTPPassThroughPB* ptp_buf = (ICAPTPPassThroughPB *) malloc(pb_size);
    memset(ptp_buf, 0, pb_size);
    ptp_buf->commandCode = command;
    ptp_buf->numOfInputParams = parms.size();
    for(size_t idx = 0; idx < parms.size(); idx += 1) {
        ptp_buf->params[idx] = parms[idx];
    }
    ptp_buf->numOfOutputParams = 0;
    ptp_buf->dataUsageMode = usage_mode;
    ptp_buf->dataSize = nsend + nrecv;

    if(nsend > 0) {
        memcpy(ptp_buf->data, send, nsend);
    }

    /* ICAError irc = */ ica_send_message_(ptp_buf, pb_size);

    uint32_t result_code = ptp_buf->resultCode;
    if(nrecv > 0) {
        memcpy(recv, ptp_buf->data, nrecv);
    }

    free(ptp_buf);
    return result_code;
}

// TODO:
ICAError MacICACamera::icaSendMessage_(void*buf, size_t buf_len)
{
    ICAObjectSendMessagePB msg;
    memset(&msg, 0, sizeof(msg));

    msg.object = dev_;
    msg.message.messageType = kICAMessageCameraPassThrough;
    msg.message.startByte = 0;
    msg.message.dataPtr = buf;
    msg.message.dataSize = buf_len;
    msg.message.dataType = kICATypeData;

    return ICAObjectSendMessage(&msg, 0);
}

int MacICACamera::batteryLevel(void)
{
    return ptpBatteryLevel();
}

int getPropertyIndex_(unsigned propCode, std::vector<PTPCamera::LabeledValue_t>& table,
                      std::vector<std::string>& values)
{
    uint32_t result_code;
    ptpProbeProperty(prop_code, result_code);

    // Get the enumeration list
    int rc = ptpGetPropertyEnum(prop_code, table);
    if(rc < 0) {
        debugLog << "MacPTPCameraControl::getPropertyIndex_: " <<
        "Property " << hex << prop_code << dec <<
        "is not an enum?!" << endl << flush;
        table.clear();
        values.clear();
        return -1;
    }

    values.resize(table.size());
    debugLog << "MacPTPCameraControl::getPropertyIndex_ " <<
    "prop=" << hex << prop_code << dec <<
    " count=" << values.size() << endl;
    for(unsigned idx = 0; idx < table.size(); idx += 1) {
        values[idx] = table[idx].label;
        debugLog << "   " << idx << ": " << values[idx].toStdString() << endl;
    }

    debugLog << flush;
    return rc;
}

void setPropertyIndex_(unsigned propCode, std::vector<PTPCamera::LabeledValue_t>& table,
                       std::vector<std::string>& values)
{
    if(use_index < 0)
        return;
    if(table.size() == 0)
        return;
    if(use_index >= table.size())
        use_index = 0;

    uint32_t rc;
    ptpSetProperty(prop_code, table[use_index].value, rc);
}

// Image size.
int MacPTPCamera::getImageSizeIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_ImageSize, imageSize_, values);
}

void MacPTPCamera::setImageSizeIndex(int index)
{
    setPropertyIndex_(PROP_ImageSize, imageSize_, index);
}

bool MacPTPCamera::setImageSizeSucceed(void)
{
    return ptpIsPropertySetable(PROP_ImageSize);
}

// Exposure program.
int MacPTPCamera::getExposureProgramIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_ExposureProgramMode, exposureProgram_, values);
}

void MacPTPCamera::setExposureProgramIndex(int index)
{
    setPropertyIndex_(PROP_ExposureProgramMode, exposureProgram_, index);
}

bool MacPTPCamera::setExposureProgramSucceed(void)
{
    return ptpIsPropertySetable(PROP_ExposureProgramMode);
}

// Exposure time, shutter speed.
int MacPTPCamera::getExposureTimeIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_ExposureTime, exposureTime_, values);
}

void MacPTPCamera::setExposureTimeIndex(int index)
{
    setExposureTimeIndex(PROP_ExposureTime, exposureTime_, index);
}

bool MacPTPCamera::setExposureTimeSucceed(void)
{
    return ptpIsPropertySetable(PROP_ExposureTime);
}

// f-number, aperture value.
int MacPTPCamera::getFNumberIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_FNumber, fNumber_, values);
}

void MacPTPCamera::setFNumberIndex(int index)
{
    setPropertyIndex_(PROP_FNumber, fNumber_, index);
}

bool MacPTPCamera::setFNumberSucceed(void)
{
    return ptpIsPropertySetable(PROP_FNumber);
}

// ISO value.
int MacPTPCamera::getISOIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_ExposureIndex, iso_, values);
}

void MacPTPCamera::setISOIndex(int index)
{
    setPropertyIndex_(PROP_ExposureIndex, iso_, index);
}

bool MacPTPCamera::setISOSucceed(void)
{
    return ptpIsPropertySetable(PROP_ExposureIndex);
}

// Flash mode.
int MacPTPCamera::getFlashModeIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_FlashMode, flashMode_, values);
}

void MacPTPCamera::setFlashModeIndex(int index)
{
    setPropertyIndex_(PROP_FlashMode, flashMode_, index);
}

bool MacPTPCamera::setFlashModeSucceed(void)
{
    return ptpIsPropertySetable(PROP_FlashMode);
}

// Focus mode.
int MacPTPCamera::getFocusModeIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_FocusMode, focusMode_, values);
}

void MacPTPCamera::setFocusModeIndex(int index)
{
    setPropertyIndex_(PROP_FocusMode, focusMode_, index);
}

bool MacPTPCamera::setFocusModeSucceed(void)
{
    return ptpIsPropertySetable(PROP_FocusMode);
}

// White balance mode.
int MacPTPCamera::getWhiteBalanceIndex(std::vector<string>& values)
{
    return getPropertyIndex_(PROP_WhiteBalance, whiteBalance_, values);
}

void MacPTPCamera::setWhiteBalanceIndex(int index)
{
    setPropertyIndex_(PROP_WhiteBalance, whiteBalance_, index);
}

bool MacPTPCamera::setWhiteBalanceSucceed(void)
{
    return ptpIsPropertySetable(PROP_WhiteBalance);
}
