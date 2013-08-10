/**
 * PTPUtil.cpp -- Realization of PTP utility.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#include "PTPUtil.h"
#include <sstream> // ostringstream
#include <cassert>

using namespace std;

#define ARRAY_ELEMENTS(x) (sizeof(x) / sizeof((x)[0]))

template<class K, class V> struct KV_T
{
    K key;
    V value;
};

template<class K, class V>
static size_t binarySearch(K key, KV_T<K, V>* table, size_t tableSize)
{
    // make sure min <= key < tableSize
    // assert(key >= 0 && tableSize > 0 && key < tableSize);

    size_t min = 0;
    size_t max = tableSize;

    while(min < max) {
        size_t mid = (min + max) / 2;
        assert(min <= mid && mid < max);

        if(table[mid].key == key)                                                                      // found
            return mid;

        if(table[mid].key < key) { // must be in upper bound
            assert(mid >= min && mid < max);
            min = mid + 1;
        } else { // must be in lower bound
            assert(mid < max);
            max = mid;
        }
    }

    assert(min == max);

    if(min == tableSize)
        return min;
    if(table[min].key != key)
        return tableSize;
    return min;
}

/**
 * This template allows the table to define the key as a pair of types.
 * Note that the types must support the <, != and == operators.
 */
template<class K1, class K2> struct KEY_PAIR
{
    K1 key1;
    K2 key2;
};

template<class K1, class K2>
static inline bool operator< (const KEY_PAIR<K1, K2>& a, const KEY_PAIR<K1, K2>& b)
{
    if(a.key1 < b.key1) return true;
    if(a.key1 > b.key1) return false;
    if(a.key2 < b.key2) return true;
    return false;
}

template<class K1, class K2>
static inline bool operator!= (const KEY_PAIR<K1, K2>& a, const KEY_PAIR<K1, K2>& b)
{
    if(a.key1 != b.key1) return true;
    if(a.key2 != b.key2) return true;
    return false;
}

template<class K1, class K2>
static inline bool operator== (const KEY_PAIR<K1, K2>& a, const KEY_PAIR<K1, K2>& b)
{
    return !(a != b);
}

/**
 * The standard opcodes listed in a contiguous range.
 * Ref: http://www.circuitsathome.com/ptpusb-control-camera-data
 */
static string S_PTP_OCS_STANDARD[] = {
    "Undefined",            // 0x1000
    "GetDeviceInfo",        // 0x1001
    "OpenSession",          // 0x1002
    "CloseSession",         // 0x1003
    "GetStorageIDs",        // 0x1004
    "GetStorageInfo",       // 0x1005
    "GetNumObjects",        // 0x1006
    "GetObjectHandles",     // 0x1007
    "GetObjectInfo",        // 0x1008
    "GetObject",            // 0x1009
    "GetThumb",             // 0x100a
    "DeteteObject",         // 0x100b
    "SendObjectInfo",       // 0x100c
    "SendObject",           // 0x100d
    "InitiateCapture",      // 0x100e
    "FormatStore",          // 0x100f
    "ResetDevice",          // 0x1010
    "SelfTest",             // 0x1011
    "SetObjectProtection",  // 0x1012
    "PowerDown",            // 0x1013
    "GetDevicePropDesc",    // 0x1014
    "GetDevicePropValue",   // 0x1015
    "SetDevicePropValue",   // 0x1016
    "ResetDevicePropValue", // 0x1017
    "TerminateOpenCapture", // 0x1018
    "MoveObject",           // 0x1019
    "CopyObject",           // 0x101a
    "GetPartialObject",     // 0x101b
    "InitiateOpenCapture"   // 0x101c
};

static KV_T<uint16_t, string> S_PTP_OCS_UNKNOWN_VENDOR[] = {
    {0xffff, ""}
};

static KV_T<uint16_t, string> S_PTP_OCS_NIKON[] = {
    { 0x90c0, "NIKON Capture" },
    { 0x90c1, "NIKON AfDrive" },
    { 0x90c2, "NIKON SetControlMode" },
    { 0x90c3, "NIKON DelImageSDRAM" },
    { 0x90c4, "NIKON 90c4" },
    { 0x90c5, "NIKON CurveDownload" },
    { 0x90c6, "NIKON CurveUpload" },
    { 0x90c7, "NIKON CheckEvent" },
    { 0x90c8, "NIKON DeviceReady" },
    { 0x90c9, "NIKON SetPreWBData" },
    { 0x90ca, "NIKON 90ca" },
    { 0x90cb, "NIKON AfCaptureSDRAM" },
    { 0x90cc, "NIKON AfCaptureSDRAM" },
    { 0x90cd, "NIKON AfCaptureSDRAM" },
    { 0x90ce, "NIKON AfCaptureSDRAM" },
    { 0x90cf, "NIKON AfCaptureSDRAM" },
    { 0x9200, "NIKON 9200" },
    { 0x9201, "NIKON 9201" },
    { 0x9202, "NIKON 9202" },
    { 0x9203, "NIKON 9203" },
    { 0x9204, "NIKON 9204" },
    { 0x9205, "NIKON 9205" },
    { 0x9206, "NIKON 9206" },
    { 0x9801, "MTP GetObjectPropsSupported" },
    { 0x9802, "MTP GetObjectPropDesc" },
    { 0x9803, "MTP GetObjectPropValue" },
    { 0x9804, "MTP SetObjectPropValue" },
    { 0x9805, "MTP GetObjPropList" }
};

static KV_T<uint16_t, string> S_PTP_OCS_CANON[] = {
    { 0x9001, "CANON GetPartialObjectInfo" },
    { 0x9002, "CANON SetObjectArchive" },
    { 0x9003, "CANON KeepDeviceOn" },
    { 0x9004, "CANON LockDeviceUI" },
    { 0x9005, "CANON UnlockDeviceUI" },
    { 0x9006, "CANON GetObjectHandleByName" },
    { 0x9008, "CANON InitiateReleaseControl" },
    { 0x9009, "CANON TerminateReleaseControl" },
    { 0x900a, "CANON TerminatePlaybackMode" },
    { 0x900b, "CANON ViewfinderOn" },
    { 0x900c, "CANON ViewfinderOff" },
    { 0x900d, "CANON DoAeAfAwb" },
    { 0x900e, "CANON GetCustomizeSpec" },
    { 0x900f, "CANON GetCustomizeItemInfo" },
    { 0x9010, "CANON GetCustomizeData" },
    { 0x9011, "CANON SetCustomizeData" },
    { 0x9012, "CANON GetCaptureStatus" },
    { 0x9013, "CANON CheckEvent" },
    { 0x9014, "CANON FocusLock" },
    { 0x9015, "CANON FocusUnlock" },
    { 0x9016, "CANON GetLocalReleaseParam" },
    { 0x9017, "CANON SetLocalReleaseParam" },
    { 0x9018, "CANON AskAboutPcEvf" },
    { 0x9019, "CANON SendPartialObject" },
    { 0x901a, "CANON InitiateCaptureInMemory" },
    { 0x901b, "CANON GetPartialObjectEx" },
    { 0x901c, "CANON SetObjectTime" },
    { 0x901d, "CANON GetViewfinderImage" },
    { 0x901e, "CANON GetObjectAttributes" },
    { 0x901f, "CANON ChangeUSBProtocol" },
    { 0x9020, "CANON GetChanges" },
    { 0x9021, "CANON GetObjectInfoEx" },
    { 0x9022, "CANON InitiateDirectTransfer" },
    { 0x9023, "CANON TerminateDirectTransfer" },
    { 0x9024, "CANON SendObjectInfoByPath" },
    { 0x9025, "CANON SendObjectByPath" },
    { 0x9026, "CANON InitiateDirectTransferEx" },
    { 0x9027, "CANON GetAncillaryObjectHandles" },
    { 0x9028, "CANON GetTreeInfo" },
    { 0x9029, "CANON GetTreeSize" },
    { 0x902a, "CANON NotifyProgress" },
    { 0x902b, "CANON NotifyCancelAccepted" },
    { 0x902d, "CANON GetDirectory" },
    { 0x9030, "CANON SetPairingInfo" },
    { 0x9031, "CANON GetPairingInfo" },
    { 0x9032, "CANON DeletePairingInfo" },
    { 0x9033, "CANON GetMACAddress" },
    { 0x9034, "CANON SetDisplayMonitor" },
    { 0x9035, "CANON PairingComplete" },
    { 0x9036, "CANON GetWirelessMAXChannel" }
};

static string S_PTP_EVENTS_STANDARD[] = {
    "Undefined",             //0x4000
    "CancelTransaction",     //0x4001
    "ObjectAdded",           //0x4002
    "ObjectRemoved",         //0x4003
    "StoreAdded",            //0x4004
    "StoreRemoved",          //0x4005
    "DevicePropChanged",     //0x4006
    "ObjectInfoChanged",     //0x4007
    "DeviceInfoChanged",     //0x4008
    "RequestObjectTransfer", //0x4009
    "StoreFull",             //0x400a
    "DeviceReset",           //0x400b
    "StoreInfoChanged",      //0x400c
    "CaptureComplete",       //0x400d
    "UnreportedStatus"       //0x400e
};

static string S_PTP_EVENTS_CANON[] = {
    "CANON 0xc000",                //0xc000
    "CANON 0xc001",                //0xc001
    "CANON 0xc002",                //0xc002
    "CANON 0xc003",                //0xc003
    "CANON 0xc004",                //0xc004
    "CANON ExtendedErrorcode",     //0xc005
    "CANON 0xc006",                //0xc006
    "CANON 0xc007",                //0xc007
    "CANON ObjectInfoChanged",     //0xc008
    "CANON RequestObjectTransfer", //0xc009
    "CANON 0xc00a",                //0xc00a
    "CANON 0xc00b",                //0xc00b
    "CANON CameraModeChanged",     //0xc00c
    "CANON 0xc00d",                //0xc00d
    "CANON 0xc00e",                //0xc00e
    "CANON 0xc00f",                //0xc00f
    "CANON 0xc010",                //0xc010
    "CANON StartDirectTransfer",   //0xc011
    "CANON 0xc012",                //0xc012
    "CANON StopDirectTransfer",    //0xc013
    "CANON 0xc019",                //0xc019
    "CANON 0xc01a"                 //0xc01a
};

static string S_PTP_EVENTS_NIKON[] = {
    "NIKON 0xc000", //0xc000
    "NIKON 0xc101", //0xc101
    "NIKON 0xc102", //0xc102
    "NIKON 0xc103", //0xc103, not supported
    "NIKON 0xc104", //0xc104
    "NIKON 0xc105", //0xc105, D5100
};

static string S_PTP_PROPERTIES_STANDARD[] = {
    "Undefined",              //0x5000
    "BatteryLevel",           //0x5001
    "FunctionalMode",         //0x5002
    "ImageSize",              //0x5003
    "CompressionSetting",     //0x5004
    "WhiteBalance",           //0x5005
    "RGB Gain",               //0x5006
    "F-Number",               //0x5007
    "FocalLength",            //0x5008
    "FocusDistance",          //0x5009
    "FocusMode",              //0x500a
    "ExposureMeteringMode",   //0x500b
    "FlashMode",              //0x500c
    "ExposureTime",           //0x500d
    "ExposureProgramMode",    //0x500e
    "ExposureIndex",          //0x500f
    "ExposureBiasCompensation", //0x5010
    "DateTime",               //0x5011
    "CaptureDelay",           //0x5012
    "StillCaptureMode",       //0x5013
    "Contrast",               //0x5014
    "Sharpness",              //0x5015
    "DigitalZoom",            //0x5016
    "EffectMode",             //0x5017
    "BurstNumber",            //0x5018
    "BurstInterval",          //0x5019
    "TimelapseNumber",        //0x501a
    "TimelapseInterval",      //0x501b
    "FocusMeteringMode",      //0x501c
    "UploadURL",              //0x501d
    "Artist",                 //0x501e
    "CopyrightInfo"           //0x501f
};

static string S_PTP_PROPERTIES_CANON[] = {
    "Undefined",          //0xd000
    "BeepMode",           //0xd001
    "BatteryKind",        //0xd002
    "BatteryStatus",      //0xd003
    "UILockType",         //0xd004
    "CameraMode",         //0xd005
    "ImageQuality",       //0xd006
    "FullViewFileFormat", //0xd007
    "ImageSize",          //0xd008
    "SelfTime",           //0xd009
    "FlashMode",          //0xd00a
    "Beep",               //0xd00b
    "ShootingMode",       //0xd00c
    "ImageMode",          //0xd00d
    "DriveMode",          //0xd00e
    "EZoom",              //0xd00f
    "MeteringMode",       //0xd010
    "AFDistance",         //0xd011
    "FocusingPoint",      //0xd012
    "WhiteBalance",       //0xd013
    "SlowShutterSetting", //0xd014
    "AFMode",             //0xd015
    "ImageStabilization", //0xd016
    "Contrast",           //0xd017
    "ColorGain",          //0xd018
    "Sharpness",          //0xd019
    "Sensitivity",        //0xd01a
    "ParameterSet",       //0xd01b
    "ISOSpeed",           //0xd01c
    "Aperture",           //0xd01d
    "ShutterSpeed",       //0xd01e
    "ExpCompensation",    //0xd01f
    "FlashCompensation",  //0xd020
    "AEBExposureCompensation", //0xd021
    "0xd022",             //0xd022
    "AvOpen",             //0xd023
    "AvMax",              //0xd024
    "FocalLength",        //0xd025
    "FocalLengthTele",  //0xd026
    "FocalLengthWide",    //0xd027
    "FocalLengthDenominator", //0xd028
    "CaptureTransferMode", //0xd029
    "Zoom",               //0xd02a
    "NamePrefix",         //0xd02b
    "SizeQualityMode",    //0xd02c
    "SupportedThumbSize", //0xd02d
    "SizeOfOutputDataFromCamera", //0xd02e
    "SizeOfInputDataToCamera",    //0xd02f
    "RemoteAPIVersion",   //0xd030
    "FirmwareVersion",    //0xd031
    "CameraModel",        //0xd032
    "CameraOwner",        //0xd033
    "UnixTime",           //0xd034
    "CameraBodyID",       //0xd035
    "CameraOutput",       //0xd036
    "DispAv",             //0xd037
    "AvOpenApex",         //0xd038
    "DZoomMagnification", //0xd039
    "MlSpotPos",          //0xd03a
    "DispAvMax",          //0xd03b
    "AvMaxApex",          //0xd03c
    "EZoomStartPosition", //0xd03d
    "FocalLengthOfTele",  //0xd03e
    "EZoomSizeOfTele",    //0xd03f
    "PhotoEffect",        //0xd040
    "AssistLight",        //0xd041
    "FlashQuantityCount", //0xd042
    "RotationAngle",      //0xd043
    "RotationScene",      //0xd044
    "EventEmulationMode", //0xd045
    "DPOFVersion",        //0xd046
    "TypeOfSupportedSlideShow", //0xd047
    "AverageFilesizes",   //0xd048
    "ModelID"             //0xd049
    //TODO: 0xd04a ~ 0xD183
};

/**
 * Use property code or operation code to retrieve property value.
 * Key  : property code | operation code (uint16_t)
 * Value: property value string
 */
static KV_T<KEY_PAIR<uint16_t, uint16_t>, string> S_PTP_PROPERTY16_VALUES_STANDARD[] = {
    // WhiteBalance
    { { 0x5005, 0x0000 }, "Undefined" },
    { { 0x5005, 0x0001 }, "Manual" },
    { { 0x5005, 0x0002 }, "Automatic" },
    { { 0x5005, 0x0003 }, "One-push Automatic" },
    { { 0x5005, 0x0004 }, "Daylight" },
    { { 0x5005, 0x0005 }, "Flourescent" },
    { { 0x5005, 0x0006 }, "Tungsten" },
    { { 0x5005, 0x0007 }, "Flash" },
    // FocusMode
    { { 0x500a, 0x0000 }, "Undefined" },
    { { 0x500a, 0x0001 }, "Manual" },
    { { 0x500a, 0x0002 }, "Automatic" },
    { { 0x500a, 0x0003 }, "Automatic/Macro" },
    // FlashMode
    { { 0x500c, 0x0000 }, "Undefined" },
    { { 0x500c, 0x0001 }, "auto flash" },
    { { 0x500c, 0x0002 }, "Flash off" },
    { { 0x500c, 0x0003 }, "Fill flash" },
    { { 0x500c, 0x0004 }, "Red eye auto" },
    { { 0x500c, 0x0005 }, "Red eye fill" },
    { { 0x500c, 0x0006 }, "External Sync" },
    // ExposureProgramMode
    { { 0x500e, 0x0000 }, "Undefined" },
    { { 0x500e, 0x0001 }, "Manual" },
    { { 0x500e, 0x0002 }, "Automatic" },
    { { 0x500e, 0x0003 }, "Aperture Priority" },
    { { 0x500e, 0x0004 }, "Shutter Priority" },
    { { 0x500e, 0x0005 }, "Program Creative" },
    { { 0x500e, 0x0006 }, "Program Action" },
    { { 0x500e, 0x0007 }, "Portait" }
    // ExposureIndex (0x500f) is handled like a range
};

static KV_T<KEY_PAIR<uint16_t, uint16_t>, string> S_PTP_PROPERTY16_VALUES_NIKON[] = {
    // WhiteBalance
    { { 0x5005, 0x8010 }, "NIKON Cloudy" },
    { { 0x5005, 0x8011 }, "NIKON Shade" },
    { { 0x5005, 0x8012 }, "NIKON Color Temperature" },
    { { 0x5005, 0x8013 }, "NIKON Preset White Balance" },
    // FlashMode
    { { 0x500c, 0x8010 }, "NIKON Fill-flash" },
    { { 0x500c, 0x8011 }, "NIKON Slow sync" },
    { { 0x500c, 0x8012 }, "NIKON Rear curtain + Slow sync" },
    { { 0x500c, 0x8013 }, "NIKON Slow sync + Red eye" },
    // ExposureProgramMode
    { { 0x500e, 0x8010 }, "NIKON Auto" },
    { { 0x500e, 0x8011 }, "NIKON Portrait" },
    { { 0x500e, 0x8012 }, "NIKON Landscape" },
    { { 0x500e, 0x8013 }, "NIKON Close up" },
    { { 0x500e, 0x8014 }, "NIKON Sports" },
    { { 0x500e, 0x8015 }, "NIKON Night portrait" },
    { { 0x500e, 0x8016 }, "NIKON Night Landscape" }
};

static KV_T<KEY_PAIR<uint16_t, uint32_t>, string> S_PTP_PROPERTY32_VALUES_STANDARD[] = {};

static KV_T<KEY_PAIR<uint16_t, uint32_t>, string> S_PTP_PROPERTY32_VALUES_NIKON[] = {};

static const uint16_t S_PTP_STANDARD_OPCODES_PREFIX = 0x1000;
static const uint16_t S_PTP_STANDARD_OPCODES_MAX = 0x001c;
static const uint16_t S_PTP_VENDOR_OPCODES_PREFIX = 0x9000;
static const uint16_t S_PTP_VENDOR_OPCODES_MAX = 0x001c;
static const uint32_t S_PTP_VENDOR_NIKON = 0x0000000a;
static const uint32_t S_PTP_VENDOR_CANON = 0x0000000b;

string ptpOpcodeString(uint16_t code, uint32_t extensionId)
{
    if( (code & 0xf000) == S_PTP_STANDARD_OPCODES_PREFIX ) { //PTP Standard Opcodes
        if((code & 0x0fff) <= S_PTP_STANDARD_OPCODES_MAX) {
            return S_PTP_OCS_STANDARD[code & 0x0fff];
        }

        ostringstream tmp;
        tmp << "Reserved-" << hex << code << ends;
        return tmp.str();
    }

    if( (code & 0xf000) == S_PTP_VENDOR_OPCODES_PREFIX ) { //Vendor Opcodes
        KV_T<uint16_t, string>* table;
        size_t tableSize = 0;
        switch(extensionId) { // TODO: make extension ID as enum ?
        case S_PTP_VENDOR_NIKON: //Nikon
            table = S_PTP_OCS_NIKON;
            tableSize = ARRAY_ELEMENTS(S_PTP_OCS_NIKON);
            break;
        case S_PTP_VENDOR_CANON: //Canon
            table = S_PTP_OCS_CANON;
            tableSize = ARRAY_ELEMENTS(S_PTP_OCS_CANON);
            break;
        default:
            table = S_PTP_OCS_UNKNOWN_VENDOR;
            tableSize = ARRAY_ELEMENTS(S_PTP_OCS_UNKNOWN_VENDOR);
            break;
        }

        size_t key = binarySearch(code, table, tableSize);
        if(key < tableSize) {
            assert(table[key].key == code);
            return table[key].value;
        }

        ostringstream tmp;
        tmp << "Vendor[" << hex << extensionId << "]-" << hex << code << ends;
        return tmp.str();
    }

    ostringstream tmp;
    tmp << "Invalid-" << hex << code << ends;
    return tmp.str();
}

static const uint16_t S_PTP_STANDARD_EVENTS_PREFIX = 0x4000;
static const uint16_t S_PTP_STANDARD_EVENTS_MAX = 0x000e;
static const uint16_t S_PTP_VENDOR_EVENTS_PREFIX = 0xc000;
string ptpEventString(uint16_t code, uint32_t vendor)
{
    if( (code & 0xf000) == S_PTP_STANDARD_EVENTS_PREFIX ) { //PTP Standard Events
        if((code & 0x0fff) <= S_PTP_STANDARD_EVENTS_MAX) {
            return S_PTP_EVENTS_STANDARD[code & 0x0fff];
        }

        ostringstream tmp;
        tmp << "Reserved-" << hex << code << ends;
        return tmp.str();
    }

    // Canon specific events.
    if( (code & 0xf000) == S_PTP_VENDOR_EVENTS_PREFIX && vendor == S_PTP_VENDOR_CANON &&
        (code & 0x0fff) < ARRAY_ELEMENTS(S_PTP_EVENTS_CANON)) {
        return S_PTP_EVENTS_CANON[code & 0x0fff];
    }

    // Nikon specific events.
    if( (code & 0xf000) == S_PTP_VENDOR_EVENTS_PREFIX && vendor == S_PTP_VENDOR_NIKON &&
        (code & 0x00ff) < ARRAY_ELEMENTS(S_PTP_EVENTS_NIKON)) {
        return S_PTP_EVENTS_NIKON[code & 0x00ff];
    }

    // Other vendor's events or undefined events by Nikon or Canon.
    if( (code & 0xf000) == S_PTP_VENDOR_EVENTS_PREFIX ) { // Other vendor events
        ostringstream tmp;
        tmp << "Vendor[" << hex << vendor << "]-" << hex << code << ends;
        return tmp.str();
    }

    ostringstream tmp;
    tmp << "Invalid-" << hex << code << ends;
    return tmp.str();
}

static const uint16_t S_PTP_STANDARD_PROPERTIES_PREFIX = 0x5000;
static const uint16_t S_PTP_STANDARD_PROPERTIES_MAX = 0x001f;
static const uint16_t S_PTP_VENDOR_PROPERTIES_PREFIX = 0xd000;
string ptpPropertyString(uint16_t code, uint32_t vendor)
{
    // PTP Standard Properties
    if( (code & 0xf000) == S_PTP_STANDARD_PROPERTIES_PREFIX ) {
        if( (code & 0x00ff) <= S_PTP_STANDARD_PROPERTIES_MAX ) {
            return S_PTP_PROPERTIES_STANDARD[code & 0x00ff];
        }

        ostringstream tmp;
        tmp << "Reserved-" << hex << code << ends;
        return tmp.str();
    }

    if( (code & 0xf000) == S_PTP_VENDOR_PROPERTIES_PREFIX && vendor == S_PTP_VENDOR_CANON &&
        (code & 0x00ff) < ARRAY_ELEMENTS(S_PTP_PROPERTIES_CANON)) {
        return S_PTP_PROPERTIES_CANON[code & 0x00ff];
    }

    // TODO: nikon?
    // Other vendor Properties
    if( (code & 0xf000) == S_PTP_VENDOR_PROPERTIES_PREFIX ) {
        ostringstream tmp;
        tmp << "Vendor-" << hex << code << ends;
        return tmp.str();
    }

    ostringstream tmp;
    tmp << "Invalid-" << hex << code << ends;
    return tmp.str();
}

string ptpPropertyUint8ToString(uint16_t code, uint8_t val, uint32_t vendor)
{
    ostringstream tmp;
    tmp << "0x" << hex << val << ends;
    return tmp.str();
}

string ptpPropertyUint16ToString(uint16_t code, uint16_t val, uint32_t vendor)
{
    KEY_PAIR<uint16_t, uint16_t> keyPair;
    keyPair.key1 = code;
    keyPair.key2 = val;

    KV_T<KEY_PAIR<uint16_t, uint16_t>, string>* table = 0;
    size_t tableSize = 0;

    if(code == 0x5007) { // F-Number
        ostringstream tmp;
        tmp << "f/" << (val / 100.0);
        return tmp.str();
    }

    if(code == 0x500f) { //ExposureIndex
        if(val == 0xffff) {
            return "Auto ISO";
        }
        ostringstream tmp;
        tmp << "ISO " << val;
        return tmp.str();
    }

    if((val & 0x8000) == 0) {
        table = S_PTP_PROPERTY16_VALUES_STANDARD;
        tableSize = ARRAY_ELEMENTS(S_PTP_PROPERTY16_VALUES_STANDARD);
        size_t key = binarySearch(keyPair, table, tableSize);

        if(key < tableSize) {
            return table[key].value;

        } else {
            ostringstream tmp;
            tmp << "Reserved-" << hex << val << ends;
            return tmp.str();
        }
    }

    switch(vendor) {
    case S_PTP_VENDOR_NIKON: //Nikon
        table = S_PTP_PROPERTY16_VALUES_NIKON;
        tableSize = ARRAY_ELEMENTS(S_PTP_PROPERTY16_VALUES_NIKON);
        break;
    default:
        table = 0;
        tableSize = 0;
        break;
    }

    size_t key = table ? binarySearch(keyPair, table, tableSize) : 0;

    if(key < tableSize) {
        return table[key].value;
    } else {
        ostringstream tmp;
        tmp << "Vendor[" << hex << vendor << "]-" << hex << val << ends;
        return tmp.str();
    }
}

string ptpPropertyUint32String(uint16_t code, uint32_t val, uint32_t vendor)
{
    KEY_PAIR<uint16_t, uint32_t> keyPair;
    keyPair.key1 = code;
    keyPair.key2 = val;

    KV_T<KEY_PAIR<uint16_t, uint32_t>, string>* table = 0;
    size_t tableSize = 0;

    if(code == 0x500d && val == 0xffffffff) { //ExposureTime
        return "Bulb";
    }

    if(code == 0x500d) { //ExposureTime
        //Exposure time is defined by PTP to be in units of 0.1ms.
        ostringstream tmp;
        tmp << (val / 10.0) << " ms" << ends;
        return tmp.str();
    }

    if((code & 0x8000) == 0) {
        table = S_PTP_PROPERTY32_VALUES_STANDARD;
        tableSize = ARRAY_ELEMENTS(S_PTP_PROPERTY32_VALUES_STANDARD);
        size_t key = binarySearch(keyPair, table, tableSize);

        if(key < tableSize) {
            return table[key].value;
        } else {
            ostringstream tmp;
            tmp << "Reserved-" << hex << val << ends;
            return tmp.str();
        }
    }

    switch(vendor) {
    case S_PTP_VENDOR_NIKON: //Nikon
        table = S_PTP_PROPERTY32_VALUES_NIKON;
        tableSize = ARRAY_ELEMENTS(S_PTP_PROPERTY32_VALUES_NIKON);
        break;
    default:
        table = 0;
        tableSize = 0;
        break;
    }

    size_t key = table ? binarySearch(keyPair, table, tableSize) : 0;

    if(key < tableSize) {
        return table[key].value;
    } else {
        ostringstream tmp;
        tmp << "Vendor[" << hex << vendor << "]-" << hex << val << ends;
        return tmp.str();
    }
}
