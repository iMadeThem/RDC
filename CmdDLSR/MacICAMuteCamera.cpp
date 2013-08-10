/**
 * MacICAMuteCamera.cpp -- Cameras that lack of capture capabilities.
 * Author: Leon (leon(at)gmail.com)
 * Date  : 2013-08-10 [Saturday, 222]
 */

#include "MacICACamera.h"
#include <iostream>
#include <cassert>

using namespace std;

MacICAMuteCamera::MacICAMuteCamera(ICAObject dev) : MacICACamera(dev)
{}

MacICAMuteCamera::~MacICAMuteCamera()
{}

CameraControl::CaptureResponse_t MacICAMuteCamera::captureImage(void)
{
    return CAP_NOT_SUPPORTED;
}
