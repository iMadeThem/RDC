/**
 * RDCCameraController.cpp -- Camera main controller class realization.
 * Author: Leon (leon(at)gmail.com)
 * Date  : 2013-08-10 [Saturday, 222]
 */

#include <iostream>

using namespace std;

void RDCCameraController::onImageAdded(RDCCamera*, const RDCCamera::FileKey_t&)
{
    debugLog << "RDCCameraController::onImageAdded " << endl << flush;

    cout << "RDCCameraController::onImageAdded" << endl;
    // TODO
}

void RDCCameraController::onImageDeleted(RDCCamera*, const RDCCamera::FileKey_t&)
{
    debugLog << "RDCCameraController::onImageDeleted " << endl << flush;

    cout << "RDCCameraController::onImageDeleted" << endl;
    // TODO
}

void RDCCameraController::onCaptureCompleted(RDCCamera*)
{
    debugLog << "RDCCameraController::onCaptureCompleted " << endl << flush;

    cout << "RDCCameraController::onCaptureCompleted" << endl;
    // TODO
}

void RDCCameraController::onCameraAttached(RDCCamera*)
{
    debugLog << "RDCCameraController::onCameraAttached " << endl << flush;

    cout << "RDCCameraController::onCameraAttached" << endl;
    // TODO
}

void RDCCameraController::onCameraDetached(RDCCamera*)
{
    debugLog << "RDCCameraController::onCameraDetached " << endl << flush;

    cout << "RDCCameraController::onCameraDetached" << endl;
    // TODO
}
