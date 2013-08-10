/**
 * RDCCameraController.h -- Camera main controller class.
 *
 * Author: Leon (leon(at)gmail.com)
 * Date  : 2013-08-10 [Saturday, 222]
 */

#ifndef _HEADER_RDCCAMERACONTROLLER_H
#define _HEADER_RDCCAMERACONTROLLER_H

#include "RDCCamera.h"
#include <fstream>
#include <string>

class RDCCamera;

class RDCCameraController : private RDCCamera::Notification
{
    public:
        RDCCameraController();
        ~RDCCameraController();

        RDCCamera* getSelectedCamera();

    private:
        RDCCamera* selectedCamera_;
        // TIMER

    private:
        // Helper methods.
        void noCameraSelected_(void);
        void displayCaptureErrorMessage_(RDCCamera::CaptureResponse_t resultCode);
        void displayBatteryLevel_(void);
        void detectCameras_(void);
        void grabCamera_(void);
        void ungrabCamera_(void);
        void displayThumbnail_(RDCCamera* camera, long imageKey);
        void writeTetheredImage_(const std::string& fileName, const BYTE* imageData);

    private:
        // Notification call backs.
        void onImageAdded(RDCCamera*, const RDCCamera::FileKey_t&);
        void onImageDeleted(RDCCamera*, const RDCCamera::FileKey_t&);
        void onCaptureCompleted(RDCCamera*);
        void onCameraAttached(RDCCamera*);
        void onCameraDetached(RDCCamera*);

};

#endif /* ~_HEADER_RDCCAMERACONTROLLER_H */
