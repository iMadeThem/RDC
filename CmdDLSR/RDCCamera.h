/**
 * RDCCamera.h -- Camera device related class, like device detection,
 * notifications.
 *
 * Author: Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-06 [Tuesday, 218]
 */

#ifndef _HEADER_RDCCAMERA_H_
#define _HEADER_RDCCAMERA_H_

#include <list>
#include <vector>
#include <map>
#include <string>
#include <ostream>
#include <fstream>

class RDCCamera
{
    public:
        RDCCamera();
        virtual ~RDCCamera() = 0;

        /* This static function scans the system for all the cameras that are
         * available. It calls the equivilent static methods in the known specific
         * classes to locate the camera devices and load them into the camera_list
         * below.
         */
        static void cameraInventory(void);

        /* Use this as a debug log for directing debug output from the entire
         * library.
         */
        static std::ofstream debugLog;

    public:
        /* Methods to get common properties of camara.
         */

        /* Get a short string that the derived class uses to identify itself.  This
         * is for identifying the calss (i.e. Image Capture Architecture, USB, etc.)
         * and not the camera itself.
         */
        virtual std::string controlClass(void) const = 0;

        /* Information about the camera. Return the make/model strings for the
         * camera.
         */
        virtual std::string maker(void) const;
        virtual std::string model(void) const;

        /* Session control related.
         * Before controlling a camera, the session should be opened first.
         * Close the session when done.
         */
        virtual int openSession(void);
        virtual int closeSession(void);

        /* Check heartbeat to assure camera existance, the client should call this
         * periodically.
         */
        virtual void heartbeat(void);

        /* Return the battery level between 0~100, -1 indicates unavailable or N/A.
         */
        virtual int batteryLevel(void);

        // Image size.
        virtual int getImageSizeIndex(std::vector<std::string>& values);
        virtual void setImageSizeIndex(int);
        virtual bool setImageSizeSucceed(void);

        // Exposure program.
        virtual int getExposureProgramIndex(std::vector<std::string>& values);
        virtual void setExposureProgramIndex(int);
        virtual bool setExposureProgramSucceed(void);

        // Exposure time, shutter speed.
        virtual int getExposureTimeIndex(std::vector<std::string>& values);
        virtual void setExposureTimeIndex(int);
        virtual bool setExposureTimeSucceed(void);

        // f-number, aperture value.
        virtual int getFNumberIndex(std::vector<std::string>& values);
        virtual void setFNumberIndex(int);
        virtual bool setFNumberSucceed(void);

        // ISO value.
        virtual int getISOIndex(std::vector<std::string>& values);
        virtual void setISOIndex(int);
        virtual bool setISOSucceed(void);

        // Flash mode.
        virtual int getFlashModeIndex(std::vector<std::string>& values);
        virtual void setFlashModeIndex(int);
        virtual bool setFlashModeSucceed(void);

        // Focus mode.
        virtual int getFocusModeIndex(std::vector<std::string>& values);
        virtual void setFocusModeIndex(int);
        virtual bool setFocusModeSucceed(void);

        // White balance mode.
        virtual int getWhiteBalanceIndex(std::vector<std::string>& values);
        virtual void setWhiteBalanceIndex(int);
        virtual bool setWhiteBalanceSucceed(void);

    public:
        /* Camera contol methods, like capturing, processing etc.
         */

        enum CaptureResponse_t {
            CAP_OK,
            CAP_NOT_SUPPORTED,
            CAP_ERROR
        };

        virtual CaptureResponse_t captureImage(void) = 0;
        virtual CaptureResponse_t captureVolatileImage(void);

    public:
        /* Image file related methods.
         *
         * Get a list of images that are known on the camera. The result reference
         * is not certain to be valid after a later call of the imageList
         * method. The reference also will not reflect changes made on the camera
         * until a new call to the image_list method refreshes the list.
         */
        typedef std::pair<long, std::string> FileKey_t;
        const std::list<FileKey_t>& imageList(void);

        /* Get the image data into a dynamically allocated buffer. The caller passes
         * in the key for the desired image. The method fills in the buf pointer and
         * the buffer len. The method allocates the memory of the buffer, it is up
         * to the caller to free the buffer when it is done.
         */
        typedef unsigned char BYTE;
        virtual void getImageByteData(long key, BYTE* data, bool deleteImage = false);
        virtual void getImageThumbnail(long key, char*& buf, size_t& bufLen);

    public:
        /* Notification.
         */
        class Notification
        {
            public:
                Notification();
                virtual ~Notification() = 0;
                virtual void onImageAdded(RDCCamera*, const FileKey_t& file);
                virtual void onImageDeleted(RDCCamera*, const FileKey_t& file);
                virtual void onCaptureCompleted(RDCCamera*);
                virtual void onCameraAttached(RDCCamera*);
                virtual void onCameraDetached(RDCCamera*);
        };

        void registerImageAddedNotification(Notification*);
        void registerImageDeletedNotification(Notification*);
        void registerCaptureCompletedNotification(Notification*);
        /* This method is static because it is used to nofify new camera, not the
         * exsiting ones.
         */
        static void registerCameraAttachedNotification(Notification*);
        void registerCameraDetachedNotification(Notification*);

    protected:
        /* Methods used for derived classes to announce events that trigger
         * notifications.
         */
        void markImageNotification(void);
        void markCaptureCompleted(void);
        static void markCameraAttached(RDCCamera*);
        static void markCameraDetached(RDCCamera*);

    private:
        void markImageAdded_(const FileKey_t& newFile);
        void markImageDeleted_(const FileKey_t& deleteFile);

    public:
        virtual void debugDump(std::ostream&, const std::string&) const;

    protected:
        /* The implementation will fill in the image file list. The method is passed
         * a reference to the list to fill with the files of the image. The
         * RDCCamera class will manage the lifetime of that list.
         */
        virtual void scanImages(std::list<FileKey_t>&) = 0;

    protected:
        /* A USB id is a vendro/device pair. */
        typedef std::pair<uint16_t, uint16_t> UsbId_t;
        /* A device name is a vendor/device string pair. */
        typedef std::pair<std::string, std::string> DeviceName_t;
        /* A device class is the derived class that supports this device. */
        enum DeviceClass_t {
            MAC_GENERIC = 0,  // Supported by default ICA basics.
            MAC_PTP,          // Supported through standard PTP.
            MAC_NOT_SUPPORTED // Positively not supported.
        };

        static void loadUsbMap(void);
        static const DeviceName_t& usbIdToName(const UsbId_t& id);
        static const DeviceClass_t& usbIdToClass(const UsbId_t& id);

    private:
        /* Internal image list. */
        std::list<FileKey_t> imageList_;
        /* Internal notification holders. */
        Notification* notificationImageAdded_;
        Notification* notificationImageDeleted_;
        Notification* notificationCaptureCompleted_;
        static Notification* notificationCameraAttached_;
        Notification* notificationCameraDetached_;
        /* Internal device IDs holder.
         * Used by the usbIdTo* methods.
         */
        static std::map<UsbId_t, DeviceName_t> usbDeviceNameMap;
        static std::map<UsbId_t, DeviceClass_t> usbDeviceClassMap;

        struct UsbDevice {
            uint16_t vendorId;
            uint16_t deviceId;
            const char* vendroName;
            const char* deviceName;
            DeviceClass_t deviceClass;
        };
        static struct UsbDevice usbDevicesTable[];

    private:
        /* No support copy constructor. */
        RDCCamera(const RDCCamera &);
        RDCCamera& operator = (const RDCCamera &);
};

/* Convenient manipulator to write a timestamp. */
extern std::ostream& TIMESTAMP(std::ostream&);

#endif /* ~_HEADER_RDCCAMERA_H_ */
