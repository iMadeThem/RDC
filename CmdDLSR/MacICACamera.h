/**
 * MacICACamera.h -- The MacICACamera class implements camera control using Mac
 * OS X's ImageCapture architecture. This is actually a category of interfaces
 * that use the Mac OS X ICA as a pass-through interfaces for advanced controls.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#ifndef _HEADER_MACICACAMERA_H_
#define _HEADER_MACICACAMERA_H_

#include "RDCCamera.h"
#include "PTPCamera.h"

//#include <Availability.h>
//#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 1070
#include <ImageCapture/ICAApplication.h> // deprecated in 10.7
#else
#include <ImageCaptureCore/ImageCaptureCore.h>
#endif
//#endif
#include <map>
#include <vector>
#include <string>
#include <inttypes.h>

class MacICACamera : public RDCCamera
{
    public:
        static void cameraInventory(void);

    protected:
        MacICACamera(ICAObject device);
    public:
        ~MacICACamera();

    public:
        std::string controlClass(void) const;
        std::string maker(void) const;
        std::string model(void) const;
        int openSession(void);
        int closeSession(void);

    public:
        CaptureResponse_t captureImage(void);
        void getImageByteData(long key, BYTE* data, bool deleteImage);
        void getImageThumbnail(long key, char*& buf, size_t& bufLen);

        void debugDump(std::ostream&, const std::string&);

    protected:
        static UsbId_t getUsbIdFromDict_(CFDictionaryRef);
        ICAError icaSendMessage_(void* buf, size_t bufLen);

        static void scanDevices_(void);

    private:
        ICAObject device_;
        CFDictionaryRef deviceDict_;
        ICASessionID icaSessionId_;
        DeviceName_t deviceModel_;

        void refreshDeviceDictionary_(void);
        static void registerForEvents(void);
        static void icaNotification(CFStringRef notificationType, CFDictionaryRef notificationDict);
        static MacICACamera* notificationCamera_;

        void scanImages(std::list<FileKey_t>&);
        ICAObject icaImageObjectFromDevice_(long key);
        void debugDumpDefault(std::ostream&) const;
        static std::ostream& dumpValue(std::ostream& out, CFTypeRef ref);
};

/**
 * This class is derived from MacICACamera for handling cameras that lack of
 * capture capabilities.
 */
class MacICABlackList : public MacICACamera
{
    private:
        friend void MacICACamera::scanDevices_(void);
        MacICABlackList(ICAObject device);

    public:
        ~MacICABlackList();

        CaptureResponse_t captureImage(void);
};

/**
 * This class is derived from PTPCamera and MacICACamera to control PTP
 * cameras. PTP cameras use Mac ICA as interface, but also used PTP commands
 * that are passed through via ICA methods.
 */
class MacPTPCamera : public PTPCamera, public MacICACamera
{
    private:
        friend void MacICACamera::scanDevices_(void);
        MacPTPCamera(ICAObject device);

    public:
        ~MacPTPCamera();

        CaptureResponse_t captureImage(void);

        virtual int batteryLevel(void);

        // Image size.
        virtual int getImageSizeIndex(std::vector<string>& values);
        virtual void setImageSizeIndex(int);
        virtual bool setImageSizeSucceed(void);

        // Exposure program.
        virtual int getExposureProgramIndex(std::vector<string>& values);
        virtual void setExposureProgramIndex(int);
        virtual bool setExposureProgramSucceed(void);

        // Exposure time, shutter speed.
        virtual int getExposureTimeIndex(std::vector<string>& values);
        virtual void setExposureTimeIndex(int);
        virtual bool setExposureTimeSucceed(void);

        // f-number, aperture value.
        virtual int getFNumberIndex(std::vector<string>& values);
        virtual void setFNumberIndex(int);
        virtual bool setFNumberSucceed(void);

        // ISO value.
        virtual int getISOIndex(std::vector<string>& values);
        virtual void setISOIndex(int);
        virtual bool setISOSucceed(void);

        // Flash mode.
        virtual int getFlashModeIndex(std::vector<string>& values);
        virtual void setFlashModeIndex(int);
        virtual bool setFlashModeSucceed(void);

        // Focus mode.
        virtual int getFocusModeIndex(std::vector<string>& values);
        virtual void setFocusModeIndex(int);
        virtual bool setFocusModeSucceed(void);

        // White balance mode.
        virtual int getWhiteBalanceIndex(std::vector<string>& values);
        virtual void setWhiteBalanceIndex(int);
        virtual bool setWhiteBalanceSucceed(void);

    private:
        int getPropertyIndex_(unsigned propCode, std::vector<PTPCamera::LabeledValue_t>& table, std::vector<std::string>& values);

        void setPropertyIndex_(unsigned propCode, std::vector<PTPCamera::LabeledValue_t>& table, std::vector<std::string>& values);

        uint32_t ptpCommand(uint16_t command, const std::vector<uint32_t>& params, const unsigned char* send, size_t sendSize, unsigned char* recv, size_t recvSize);

    private:
        // standard camera properties
        std::vector<PTPCamera::LabeledValue_t> imageSize_;
        std::vector<PTPCamera::LabeledValue_t> whiteBalance_;
        std::vector<PTPCamera::LabeledValue_t> exposureProgram_;
        std::vector<PTPCamera::LabeledValue_t> exposureTime_;
        std::vector<PTPCamera::LabeledValue_t> fNumber_;
        std::vector<PTPCamera::LabeledValue_t> iso_;
        std::vector<PTPCamera::LabeledValue_t> flashMode_;
        std::vector<PTPCamera::LabeledValue_t> focusMode_;
};

#endif /* ~_HEADER_MACICACAMERA_H_ */
