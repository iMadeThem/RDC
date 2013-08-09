/**
 * PTPCamera.cpp -- PTP camera realization.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#include "PTPCamera.h"
#include "PTPUtil.h"
#include "RDCCamera.h"
#include <stddef.h>
#include <cassert>
#include <sstream>

using namespace std;

static map<uint16_t, PTPCamera::TypeCode_t> generateTypeCodeMap(void)
{
    map<uint16_t, PTPCamera::TypeCode_t> tmp;
    tmp[PTPCamera::TYPE_NONE] = PTPCamera::TYPE_NONE;
    tmp[PTPCamera::TYPE_INT8] = PTPCamera::TYPE_INT8;
    tmp[PTPCamera::TYPE_UINT8] = PTPCamera::TYPE_UINT8;
    tmp[PTPCamera::TYPE_INT16] = PTPCamera::TYPE_INT16;
    tmp[PTPCamera::TYPE_UINT16] = PTPCamera::TYPE_UINT16;
    tmp[PTPCamera::TYPE_INT32] = PTPCamera::TYPE_INT32;
    tmp[PTPCamera::TYPE_UINT32] = PTPCamera::TYPE_UINT32;
    tmp[PTPCamera::TYPE_INT64] = PTPCamera::TYPE_INT64;
    tmp[PTPCamera::TYPE_UINT64] = PTPCamera::TYPE_UINT64;
    tmp[PTPCamera::TYPE_INT128] = PTPCamera::TYPE_INT128;
    tmp[PTPCamera::TYPE_UINT128] = PTPCamera::TYPE_UINT128;
    tmp[PTPCamera::TYPE_STRING] = PTPCamera::TYPE_STRING;
    return tmp;
}

map<uint16_t, PTPCamera::TypeCode_t> PTPCamera::s_PtpTypeToTypeCode_ = generateTypeCodeMap();

template<class T> static T valueFromBytes(unsigned char*& buf);

template<> static int8_t valueFromBytes<int8_t>(unsigned char*& buf)
{
    int8_t val = (int8_t) buf[0];
    buf += 1;
    return val;
}

template<> static uint8_t valueFromBytes<uint8_t>(unsigned char*& buf)
{
    uint8_t val = (uint8_t) buf[0];
    buf += 1;
    return val;
}

template<> static int16_t valueFromBytes<int16_t>(unsigned char*& buf)
{
    uint16_t val = (uint16_t) buf[1];
    val <<= 8;
    val |= (uint16_t) buf[0];
    buf += 2;
    return (int16_t) val;
}

template<> static uint16_t valueFromBytes<uint16_t>(unsigned char*& buf)
{
    uint16_t val = (uint16_t) buf[1];
    val <<= 8;
    val |= (uint16_t) buf[0];
    buf += 2;
    return val;
}

template<> static int32_t valueFromBytes<int32_t>(unsigned char*& buf)
{
    uint32_t val = (uint32_t) buf[3];
    val <<= 8;
    val |= (uint32_t) buf[2];
    val <<= 8;
    val |= (uint32_t) buf[1];
    val <<= 8;
    val |= (uint32_t) buf[0];
    buf += 4;
    return (int32_t) val;
}

template<> static uint32_t valueFromBytes<uint32_t>(unsigned char*& buf)
{
    uint32_t val = (uint32_t) buf[3];
    val <<= 8;
    val |= (uint32_t) buf[2];
    val <<= 8;
    val |= (uint32_t) buf[1];
    val <<= 8;
    val |= (uint32_t) buf[0];
    buf += 4;
    return val;
}

template<> static vector<uint16_t> valueFromBytes<vector<uint16_t>>(unsigned char*& buf)
{
    uint32_t size = valueFromBytes<uint32_t>(buf);
    vector<uint16_t> val(size);
    for(size_t idx = 0; idx < val.size(); idx += 1)
        val[idx] = valueFromBytes<uint16_t>(buf);

    return val;
}

template<> static string valueFromBytes<string>(unsigned char*& buf)
{
    string result;
    uint8_t slen = valueFromBytes<uint8_t>(buf);
    if(slen > 0) {
        unsigned short* str = new unsigned short[slen];
        for(uint8_t idx = 0; idx < slen; idx += 1)
            str[idx] = valueFromBytes<uint16_t>(buf);
        //	    result.assign(str); // TODO: convert unsigned short arrray to string
        delete[] str;
    }
    return result;
}

const PTPCamera::PropertyValue_t PTPCamera::s_propertyValueNil;

string PTPCamera::resultCode(uint32_t rc)
{
    string rcStr;
    // rcStr = to_string(rc);// "to_string" is added in C++11
    stringstream tmp;
    tmp << "0x" << hex << rc;
    rcStr = tmp.str();

    switch(rc) {
    case 0x2001:
        rcStr.append(" (OK)");
        break;
    case 0x2002:
        rcStr.append(" (General Error)");
        break;
    case 0x2003:
        rcStr.append(" (Session Not Open)");
        break;
    case 0x2005:
        rcStr.append(" (Operation Not Supported)");
        break;
    default:
        break;
    }

    return rcStr;
}

PTPCamera::PTPCamera()
{}

PTPCamera::~PTPCamera()
{}

uint32_t PTPCamera::ptpGetDeviceInfo(void)
{
    unsigned char recv_buf[1024];

    uint32_t rc = ptpCommand(0x1001 /* GetDeviceInfo */, vector<uint32_t>(),
                             0, 0, recv_buf, sizeof(recv_buf));

    unsigned char* dptr = recv_buf;
    vector<uint16_t> properties_list;

    ptpStandardVersion_ = valueFromBytes<uint16_t>(dptr);
    vendorExtensionId_ = valueFromBytes<uint32_t>(dptr);
    vendorExtensionVersion_ = valueFromBytes<uint16_t>(dptr);
    vendorExtensionDescription_ = valueFromBytes<string>(dptr);
    functionalMode_ = valueFromBytes<uint16_t>(dptr);
    operationsSupported_ = valueFromBytes<vector<uint16_t>>(dptr);
    eventsSupported_ = valueFromBytes<vector<uint16_t>>(dptr);
    properties_list = valueFromBytes<vector<uint16_t>>(dptr);
    captureFormats_ = valueFromBytes<vector<uint16_t>>(dptr);
    imageFormats_ = valueFromBytes<vector<uint16_t>>(dptr);
    deviceManufacturer_ = valueFromBytes<string>(dptr);
    deviceModel_ = valueFromBytes<string>(dptr);
    deviceVersion_ = valueFromBytes<string>(dptr);
    serialNumber_ = valueFromBytes<string>(dptr);

    devicePropertiesSupportedMap_.clear();
    for(size_t idx = 0; idx < properties_list.size(); idx += 1) {
        PropertyInfo_t tmp;
        devicePropertiesSupportedMap_[properties_list[idx]] = tmp;
    }

    return rc;
}

uint16_t PTPCamera::ptpGetStandardVersion() const
{
    return ptpStandardVersion_;
}

uint32_t PTPCamera::ptpGetExtensionVendor(bool raw_id) const
{
    if(raw_id) return vendorExtensionId_;

    uint32_t use_extension_id = vendorExtensionId_;
    // Some Nikon cameras forgot to set their vendor extension ID
    // to Nikon, and instead report themselves as "Microsoft." So
    // define a practical extension id to use.
    if(use_extension_id == 0x006 && ptpManufacturer() == "Nikon") {
        use_extension_id = 0x000a;
    }

    return use_extension_id;
}

uint16_t PTPCamera::ptpGetExtensionVersion() const
{
    return vendorExtensionVersion_;
}

string PTPCamera::ptpGetExtensionDescription() const
{
    return vendorExtensionDescription_;
}

bool PTPCamera::ptpIsOperationCodeSupported(uint16_t code) const
{
    for(size_t idx = 0; idx < operationsSupported_.size(); idx += 1) {
        if(code == operationsSupported_[idx]) return true;
    }

    return false;
}

uint16_t PTPCamera::ptpFunctionalMode() const
{
    return functionalMode_;
}

string PTPCamera::ptpManufacturer() const
{
    return deviceManufacturer_;
}

string PTPCamera::ptpModel() const
{
    return deviceModel_;
}

string PTPCamera::ptpDeviceVersion() const
{
    return deviceVersion_;
}

string PTPCamera::ptpSerialNumber() const
{
    return serialNumber_;
}

float PTPCamera::ptpBatteryLevel()
{
    uint32_t rc;
    // Get the PTP standard BatteryLevel property value.
    PropertyValue_t val = ptpGetCameraProperty(0x5001 /* PTP BatteryLevel */, rc);

    if(val.getType() == TYPE_NONE)
        return -1;

    // The BatteryLevel is by definition (PTP) a UINT8.
    assert(val.getType() == TYPE_UINT8);

    map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(0x5001);
    assert(cur != devicePropertiesSupportedMap_.end());

    uint8_t val_min, val_max;
    if(cur->second.formFlag == 1) { // RANGE
        val_min = cur->second.range[0].getUint8();
        val_max = cur->second.range[1].getUint8();

    } else if(cur->second.formFlag == 2) { // ENUM
        val_min = cur->second.range[0].getUint8();
        val_max = val_min;
        for(size_t idx = 1; idx < cur->second.range.size(); idx += 1) {
            uint8_t tmp = cur->second.range[idx].getUint8();
            if(tmp < val_min)
                val_min = tmp;
            if(tmp > val_max)
                val_max = tmp;
        }
    }

    uint8_t use_val = val.getUint8();
    if(use_val >= val_max)
        return 100.0;
    if(use_val <= val_min)
        return 0.0;

    use_val -= val_min;
    val_max -= val_min;
    return (use_val * 100.0) / (val_max * 1.0);
}

vector<string> PTPCamera::ptpGetOperationsList() const
{
    uint32_t use_extension_id = ptpGetExtensionVendor();
    vector<string> res (operationsSupported_.size());

    for(unsigned idx = 0; idx < operationsSupported_.size(); idx += 1) {
        string opcode_string = ptpOpcodeString(operationsSupported_[idx],
                                               use_extension_id);
        res[idx] = opcode_string.c_str();
    }

    return res;
}

vector<string> PTPCamera::ptpGetEventsList() const
{
    uint32_t use_extension_id = ptpGetExtensionVendor();
    vector<string> res (eventsSupported_.size());

    for(unsigned idx = 0; idx < eventsSupported_.size(); idx += 1) {
        string event_string = ptpEventString(eventsSupported_[idx],
                                             use_extension_id);
        res[idx] = event_string.c_str();
    }

    return res;
}

vector<pair<uint16_t, string>> PTPCamera::ptpGetPropertiesList() const
{
    uint32_t use_extension_id = ptpGetExtensionVendor();
    vector<CodeStringPair_t> res(devicePropertiesSupportedMap_.size());

    size_t idx = 0;
    for(map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.begin()
        ; cur != devicePropertiesSupportedMap_.end(); cur++) {
        string prop_string = ptpPropertyString(cur->first, use_extension_id);
        pair<uint16_t, string> item;
        item.first = cur->first;
        item.second = prop_string.c_str();
        res[idx++] = item;
    }

    return res;
}

static string mapImageFormatToString(uint16_t code)
{
    switch(code) {
    case 0x3000:
        return "Undefined non-image object";
    case 0x3001:
        return "Association (e.g. directory)";
    case 0x3002:
        return "Script (device-model specific)";
    case 0x3006:
        return "Digital Print Order Format (text)";
    case 0x3800:
        return "Unknown image object";
    case 0x3801:
        return "EXIF/JPEG";
    case 0x3808:
        return "JFIF";
    case 0x380d:
        return "TIFF";
    default:
        ostringstream tmp;
        tmp << "0x" << hex << code;
        return tmp.str();
    }
}

vector<string> PTPCamera::ptpGetCaptureFormatsList() const
{
    vector<string> res(captureFormats_.size());

    for(unsigned idx = 0; idx < captureFormats_.size(); idx += 1) {
        res[idx] = mapImageFormatToString(captureFormats_[idx]);
    }

    return res;
}

vector<string> PTPCamera::ptpGetImageFormatsList() const
{
    vector<string> res(imageFormats_.size());

    for(unsigned idx = 0; idx < imageFormats_.size(); idx += 1) {
        res[idx] = mapImageFormatToString(imageFormats_[idx]);
    }

    return res;
}

PTPCamera::TypeCode_t PTPCamera::ptpGetPropertyTypeCode(unsigned prop_code) const
{
    std::map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end())
        return TYPE_NONE;
    else
        return cur->second.typeCode;
}

bool PTPCamera::ptpIsPropertySetable(unsigned prop_code) const
{
    std::map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end())
        return false;

    if(cur->second.getSetFlag)
        return true;
    else
        return false;
}

PTPCamera::TypeForm_t PTPCamera::ptpGetPropertyTypeForm(unsigned prop_code) const
{
    std::map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end())
        return FORM_NONE;

    if(cur->second.formFlag == FORM_ENUM)
        return FORM_ENUM;
    else
        return FORM_RANGE;
}

int PTPCamera::ptpGetPropertyEnum(unsigned prop_code, vector<LabeledValue_t>& table) const
{
    table.clear();
    map<uint16_t, PropertyInfo_t>::const_iterator info = devicePropertiesSupportedMap_.find(prop_code);
    if(info == devicePropertiesSupportedMap_.end())
        return -1;

    if(info->second.formFlag != 2)                                     // ENUM
        return -1;

    table.resize(info->second.range.size());
    switch(info->second.typeCode) {
    case TYPE_STRING:
        for(size_t idx = 0; idx < table.size(); idx += 1) {
            table[idx].label = info->second.range[idx].getString();
            table[idx].value = info->second.range[idx];
        }
        break;
    case TYPE_UINT8:
        for(size_t idx = 0; idx < table.size(); idx += 1) {
            uint8_t val = info->second.range[idx].getUint8();
            string tmp = ptpPropertyUint8String(prop_code, val,
                                                ptpGetExtensionVendor());
            table[idx].label = string(tmp.c_str());
            table[idx].value = info->second.range[idx];
        }
        break;
    case TYPE_UINT16:
        for(size_t idx = 0; idx < table.size(); idx += 1) {
            uint16_t val = info->second.range[idx].getUint16();
            string tmp = ptpPropertyUint16String(prop_code, val,
                                                 ptpGetExtensionVendor());
            table[idx].label = string(tmp.c_str());
            table[idx].value = info->second.range[idx];
        }
        break;
    case TYPE_UINT32:
        for(size_t idx = 0; idx < table.size(); idx += 1) {
            uint32_t val = info->second.range[idx].getUint32();
            string tmp = ptpPropertyUint32String(prop_code, val,
                                                 ptpGetExtensionVendor());
            table[idx].label = string(tmp.c_str());
            table[idx].value = info->second.range[idx];
        }
        break;
    default:
        for(size_t idx = 0; idx < table.size(); idx += 1) {
            table[idx].value = info->second.range[idx];
        }
        break;
    }

    int cur_idx = 0;
    for(size_t idx = 0; idx < table.size(); idx += 1) {
        if(info->second.current == table[idx].value) {
            cur_idx = idx;
            break;
        }
    }

    return cur_idx;
}

bool PTPCamera::ptpGetPropertyIsRange(unsigned prop_code, PropertyValue_t& min, PropertyValue_t& max, PropertyValue_t& step) const
{
    map<uint16_t, PropertyInfo_t>::const_iterator info = devicePropertiesSupportedMap_.find(prop_code);
    if(info == devicePropertiesSupportedMap_.end())
        return false;

    if(info->second.formFlag != 1)                                     // This method only works with RANGE types
        return false;

    assert(info->second.range.size() == 3);

    min = info->second.range[0];
    max = info->second.range[1];
    step = info->second.range[2];
    return true;
}

const PTPCamera::PropertyValue_t& PTPCamera::ptpGetPropertyCurrent(unsigned prop_code) const
{
    map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end())
        return s_propertyValueNil;

    return cur->second.current;
}

const PTPCamera::PropertyValue_t& PTPCamera::ptpGetPropertyFactory(unsigned prop_code) const
{
    map<uint16_t, PropertyInfo_t>::const_iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end())
        return s_propertyValueNil;

    return cur->second.factory;
}

int8_t PTPCamera::ptpGetPropertyInt8_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[1];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);
    return recv_buf[0];
}

uint8_t PTPCamera::ptpGetPropertyUint8_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[1];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);
    return recv_buf[0];
}

int16_t PTPCamera::ptpGetPropertyInt16_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[2];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    int16_t val = recv_buf[1];
    val <<= 8;
    val |= recv_buf[0];
    return val;
}

uint16_t PTPCamera::ptpGetPropertyUint16_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[2];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    uint16_t val = recv_buf[1];
    val <<= 8;
    val |= recv_buf[0];
    return val;
}

int32_t PTPCamera::ptpGetPropertyInt32_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[4];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    uint32_t val = recv_buf[3];
    val <<= 8; val |= recv_buf[2];
    val <<= 8; val |= recv_buf[1];
    val <<= 8; val |= recv_buf[0];
    return val;
}

uint32_t PTPCamera::ptpGetPropertyUint32_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[4];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    uint32_t val = recv_buf[3];
    val <<= 8; val |= recv_buf[2];
    val <<= 8; val |= recv_buf[1];
    val <<= 8; val |= recv_buf[0];
    return val;
}

string PTPCamera::ptpGetPropertyString_(unsigned prop_code, uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char recv_buf[512];

    params[0] = prop_code;
    result_code = ptpCommand(0x1015 /* GetDevicePropValue */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    unsigned char*dptr = recv_buf;
    return valueFromBytes<string>(dptr);
}

const PTPCamera::PropertyValue_t& PTPCamera::ptpGetCameraProperty(unsigned prop_code, uint32_t&rc)
{
    map<uint16_t, PropertyInfo_t>::iterator cur = devicePropertiesSupportedMap_.find(prop_code);
    if(cur == devicePropertiesSupportedMap_.end()) {
        RDCCamera::debugLog << "PTPCamera::ptpGetCameraProperty(" <<
        hex << prop_code << dec <<
        "): Property not found?" << endl << flush;
        rc = 0;
        return s_propertyValueNil;
    }

    if(cur->second.current.getType() == TYPE_NONE) {
        RDCCamera::debugLog << "PTPCamera::ptpGetCameraProperty(" <<
        hex << prop_code << dec <<
        "): Probe property" << endl << flush;
        ptpProbeProperty(prop_code, rc);
    }

    switch(cur->second.typeCode) {
    case TYPE_NONE:
        cur->second.current = s_propertyValueNil;
        break;
    case TYPE_INT8:
        cur->second.current.setInt8( ptpGetPropertyInt8_(prop_code, rc) );
        break;
    case TYPE_UINT8:
        cur->second.current.setUint8( ptpGetPropertyUint8_(prop_code, rc) );
        break;
    case TYPE_INT16:
        cur->second.current.setInt16( ptpGetPropertyInt16_(prop_code, rc) );
        break;
    case TYPE_UINT16:
        cur->second.current.setUint16( ptpGetPropertyUint16_(prop_code, rc) );
        break;
    case TYPE_INT32:
        cur->second.current.setInt32( ptpGetPropertyInt32_(prop_code, rc) );
        break;
    case TYPE_UINT32:
        cur->second.current.setUint32( ptpGetPropertyUint32_(prop_code, rc) );
        break;
    case TYPE_STRING:
        cur->second.current.setString( ptpGetPropertyString_(prop_code, rc) );
        break;
    case TYPE_INT64:
    case TYPE_UINT64:
    case TYPE_INT128:
    case TYPE_UINT128:
        assert(0);
        break;
    }

    return cur->second.current;
}

void PTPCamera::ptpSetPropertyInt8_(unsigned prop_code, int8_t val,
                                    uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[1];

    send_buf[0] = val;

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);
}

void PTPCamera::ptpSetPropertyUint8_(unsigned prop_code, uint8_t val,
                                     uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[1];

    send_buf[0] = val;

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);
}

void PTPCamera::ptpSetPropertyInt16_(unsigned prop_code, int16_t val,
                                     uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[2];

    send_buf[0] = (val >> 0) & 0xff;
    send_buf[1] = (val >> 8) & 0xff;

    RDCCamera::debugLog << "PTPCamera::ptpSetPropertyInt16_(" <<
    "prop=0x" << hex << prop_code <<
    ", val=0x" << hex << val << dec << ")";

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);

    RDCCamera::debugLog << " --> 0x" << hex << result_code << dec << endl;
}

void PTPCamera::ptpSetPropertyUint16_(unsigned prop_code, uint16_t val,
                                      uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[2];

    send_buf[0] = (val >> 0) & 0xff;
    send_buf[1] = (val >> 8) & 0xff;

    RDCCamera::debugLog << "PTPCamera::ptpSetPropertyUint16_(" <<
    "prop=0x" << hex << prop_code <<
    ", val=0x" << hex << val << dec << ")";

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);

    RDCCamera::debugLog << " --> 0x" << hex << result_code << dec << endl;
}

void PTPCamera::ptpSetPropertyInt32_(unsigned prop_code, int32_t val,
                                     uint32_t&result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[4];

    send_buf[0] = (val >> 0) & 0xff;
    send_buf[1] = (val >> 8) & 0xff;
    send_buf[2] = (val >> 16) & 0xff;
    send_buf[3] = (val >> 24) & 0xff;

    RDCCamera::debugLog << "PTPCamera::ptpSetPropertyInt32_(" <<
    "prop=0x" << hex << prop_code <<
    ", val=0x" << hex << val << dec << ")";

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);

    RDCCamera::debugLog << " --> 0x" << hex << result_code << dec << endl;
}

void PTPCamera::ptpSetPropertyUint32_(unsigned prop_code, uint32_t val,
                                      uint32_t& result_code)
{
    vector<uint32_t> params(1);
    unsigned char send_buf[4];

    send_buf[0] = (val >> 0) & 0xff;
    send_buf[1] = (val >> 8) & 0xff;
    send_buf[2] = (val >> 16) & 0xff;
    send_buf[3] = (val >> 24) & 0xff;

    RDCCamera::debugLog << "PTPCamera::ptpSetPropertyUint32_(" <<
    "prop=0x" << hex << prop_code <<
    ", val=0x" << hex << val << dec << ")";

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, sizeof send_buf, 0, 0);

    RDCCamera::debugLog << " --> 0x" << hex << result_code << dec << endl;
}

void PTPCamera::ptpSetPropertyString_(unsigned prop_code, const string& val, uint32_t& result_code)
{
    vector<uint32_t> params(1);
    size_t send_siz = val.length();
    const unsigned char* send_buf = (const unsigned char *) val.c_str();

    params[0] = prop_code;
    result_code = ptpCommand(0x1016 /* SetDevicePropValue */, params,
                             send_buf, send_siz, 0, 0);
}

void PTPCamera::ptpSetProperty(unsigned prop_code, const PropertyValue_t&val, uint32_t&result_code)
{
    switch(val.getType()) {
    case TYPE_NONE:
        break;
    case TYPE_INT8:
        ptpSetPropertyInt8_(prop_code, val.getUint8(), result_code);
        break;
    case TYPE_UINT8:
        ptpSetPropertyUint8_(prop_code, val.getUint8(), result_code);
        break;
    case TYPE_INT16:
        ptpSetPropertyInt16_(prop_code, val.getUint16(), result_code);
        break;
    case TYPE_UINT16:
        ptpSetPropertyUint16_(prop_code, val.getUint16(), result_code);
        break;
    case TYPE_INT32:
        ptpSetPropertyInt32_(prop_code, val.getUint32(), result_code);
        break;
    case TYPE_UINT32:
        ptpSetPropertyUint32_(prop_code, val.getUint32(), result_code);
        break;
    case TYPE_STRING:
        ptpSetPropertyString_(prop_code, val.getString(), result_code);
        break;

    default:
        assert(0);
    }
}

void PTPCamera::ptpProbeProperty(unsigned prop_code, uint32_t& result_code)
{
    // First get a reference to the entry for this property in the
    // support table.
    std::map<uint16_t, PropertyInfo_t>::iterator desc = devicePropertiesSupportedMap_.find(prop_code);
    if(desc == devicePropertiesSupportedMap_.end()) {
        return;
    }

    // Send a command to the device to describe this property.
    unsigned char recv_buf[1024];
    vector<uint32_t> params(1);
    params[0] = prop_code;
    result_code = ptpCommand(0x1014 /* GetDevicePropDesc */, params,
                             0, 0, recv_buf, sizeof recv_buf);

    RDCCamera::debugLog << "GetDevicePropDesc(" << hex << prop_code <<
    ") result_code=" << hex << result_code <<
    dec << endl;

    if(result_code != 0x2001)
        return;

    unsigned char*dptr = recv_buf;

    // data[0]
    // data[1] -- Property code back
    uint16_t prop = valueFromBytes<uint16_t>(dptr);
    assert(prop == prop_code);

    // data[2]
    // data[3] -- data type code
    // Setting the type code for the property also turns its
    // support flag on.
    uint16_t type = valueFromBytes<uint16_t>(dptr);
    desc->second.typeCode = s_PtpTypeToTypeCode_[type];

    // data[4] -- GetSet flag
    desc->second.getSetFlag = valueFromBytes<uint8_t>(dptr);

    RDCCamera::debugLog << "    prop: 0x" << hex << prop << dec << endl;
    RDCCamera::debugLog << "    type: 0x" << hex << type << dec << endl;
    RDCCamera::debugLog << "  GetSet: " << (int) desc->second.getSetFlag << endl;

    // Starting at data[5]...
    //   -- Factory Default value
    //   -- Current value
    switch(desc->second.typeCode) {
    case 0:  // UNDEFINED
    case 1:  // INT8
        desc->second.factory.setInt8(valueFromBytes<int8_t>(dptr));
        desc->second.current.setInt8(valueFromBytes<int8_t>(dptr));
        break;
    case 2:  // UINT8
        desc->second.factory.setUint8(valueFromBytes<uint8_t>(dptr));
        desc->second.current.setUint8(valueFromBytes<uint8_t>(dptr));
        break;
    case 3:  // INT16
        desc->second.factory.setInt16(valueFromBytes<int16_t>(dptr));
        desc->second.current.setInt16(valueFromBytes<int16_t>(dptr));
        break;
    case 4:  // UINT16
        desc->second.factory.setUint16(valueFromBytes<uint16_t>(dptr));
        desc->second.current.setUint16(valueFromBytes<uint16_t>(dptr));
        break;
    case 5:  // INT32
        desc->second.factory.setInt32(valueFromBytes<int32_t>(dptr));
        desc->second.current.setInt32(valueFromBytes<int32_t>(dptr));
        break;
    case 6:  // UINT32
        desc->second.factory.setUint32(valueFromBytes<uint32_t>(dptr));
        desc->second.current.setUint32(valueFromBytes<uint32_t>(dptr));
        break;
    case 7:  // INT64
    case 8:  // UINT64
    case 9:  // INT128;
    case 10: // UINT128;
        break;
    case 0xffff: // String
        desc->second.factory.setString(valueFromBytes<string>(dptr));
        desc->second.current.setString(valueFromBytes<string>(dptr));
        break;
    default:
        break;
    }

    // The form flag...
    desc->second.formFlag = valueFromBytes<uint8_t>(dptr);

    RDCCamera::debugLog << "    form: " << (int) desc->second.formFlag << endl;

    if(desc->second.formFlag == 1) { // RANGE
        // The range description includes 3 values: the minimum
        // value, the maximum value and the step.
        desc->second.range = vector<PropertyValue_t>(3);
        switch(desc->second.typeCode) {
        case 1: { // INT8
            desc->second.range[0].setInt8(valueFromBytes<int8_t>(dptr));
            desc->second.range[1].setInt8(valueFromBytes<int8_t>(dptr));
            desc->second.range[2].setInt8(valueFromBytes<int8_t>(dptr));
            break;
        }
        case 2: { // UINT8
            desc->second.range[0].setUint8(valueFromBytes<uint8_t>(dptr));
            desc->second.range[1].setUint8(valueFromBytes<uint8_t>(dptr));
            desc->second.range[2].setUint8(valueFromBytes<uint8_t>(dptr));
            break;
        }
        default: {
            break;
        }
        }

    } else if(desc->second.formFlag == 2) { // ENUM
        // An enumeration is a complete list of the possible
        // value that the property can take.
        uint16_t count = valueFromBytes<uint16_t>(dptr);
        desc->second.range = vector<PropertyValue_t>(count);

        switch(desc->second.typeCode) {
        case 1: // INT8
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setInt8(valueFromBytes<int8_t>(dptr));
            break;
        case 2: // UINT8
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setUint8(valueFromBytes<uint8_t>(dptr));
            break;
        case 3: // INT16
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setInt16(valueFromBytes<int16_t>(dptr));
            break;
        case 4: // UINT16
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setUint16(valueFromBytes<uint16_t>(dptr));
            break;
        case 5: // INT32
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setInt32(valueFromBytes<int32_t>(dptr));
            break;
        case 6: // UINT32
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setUint32(valueFromBytes<uint32_t>(dptr));
            break;
        case 0xffff: // String
            for(unsigned idx = 0; idx < count; idx += 1)
                desc->second.range[idx].setString(valueFromBytes<string>(dptr));
            break;
        default:
            break;
        }
    } else {}
}

bool PTPCamera::ptpInitiateCapture(uint32_t&rc)
{
    if(!ptpIsOperationCodeSupported(0x100e))
        return false;

    vector<uint32_t> params(2);
    params[0] = 0x00000000; // StorageId
    params[1] = 0x00000000; // ObjectFormatCode
    rc = ptpCommand(0x100e /* InitiateCapture */, params, 0, 0, 0, 0);

    return true;
}

PTPCamera::PropertyValue_t::PropertyValue_t() :
    typeCode_(PTPCamera::TYPE_NONE)
{}

PTPCamera::PropertyValue_t::PropertyValue_t(const PropertyValue_t& that)
{
    copy_(that);
}

PTPCamera::PropertyValue_t& PTPCamera::PropertyValue_t::operator= (const PropertyValue_t& that)
{
    if(this != &that) copy_(that);
    return *this;
}

void PTPCamera::PropertyValue_t::copy_(const PropertyValue_t& that)
{
    typeCode_ = that.typeCode_;
    switch(typeCode_) {
    case 0:
        break;
    case 1: // INT8
        valInt8_ = that.valInt8_;
        break;
    case 2: // UINT8
        valUint8_ = that.valUint8_;
        break;
    case 3: // INT16
        valInt16_ = that.valInt16_;
        break;
    case 4: // UINT16
        valUint16_ = that.valUint16_;
        break;
    case 5: // INT32
        valInt32_ = that.valInt32_;
        break;
    case 6: // UINT32
        valUint32_ = that.valUint32_;
        break;
    case 0xffff: // String
        valString_ = new string(*that.valString_);
        break;
    default:
        assert(0);
    }
}

bool PTPCamera::PropertyValue_t::operator == (const PTPCamera::PropertyValue_t&that) const
{
    if(typeCode_ != that.typeCode_)
        return false;

    switch(typeCode_) {
    case TYPE_NONE:
        return true;
    case TYPE_INT8:
        return getInt8() == that.getInt8();
    case TYPE_UINT8:
        return getUint8() == that.getUint8();
    case TYPE_INT16:
        return getInt16() == that.getInt16();
    case TYPE_UINT16:
        return getUint16() == that.getUint16();
    case TYPE_INT32:
        return getInt32() == that.getInt32();
    case TYPE_UINT32:
        return getUint32() == that.getUint32();
    case TYPE_INT64:
        return getInt64() == that.getInt64();
    case TYPE_UINT64:
        return getUint64() == that.getUint64();
    case TYPE_INT128:
        return getInt128() == that.getInt128();
    case TYPE_UINT128:
        return getUint128() == that.getUint128();
    case TYPE_STRING:
        return getString() == that.getString();
    }

    return false;
}

void PTPCamera::PropertyValue_t::clear()
{
    switch(typeCode_) {
    case 0xffff: // String
        delete valString_;
        break;
    default:
        break;
    }
    typeCode_ = TYPE_NONE;
}

PTPCamera::PropertyValue_t::~PropertyValue_t()
{
    clear();
}

int8_t PTPCamera::PropertyValue_t::getInt8(void) const
{
    assert(typeCode_ == TYPE_INT8);
    return valInt8_;
}

uint8_t PTPCamera::PropertyValue_t::getUint8(void) const
{
    assert(typeCode_ == TYPE_UINT8);
    return valUint8_;
}

int16_t PTPCamera::PropertyValue_t::getInt16(void) const
{
    assert(typeCode_ == TYPE_INT16);
    return valInt16_;
}

uint16_t PTPCamera::PropertyValue_t::getUint16(void) const
{
    assert(typeCode_ == TYPE_UINT16);
    return valUint16_;
}

int32_t PTPCamera::PropertyValue_t::getInt32(void) const
{
    assert(typeCode_ == TYPE_INT32);
    return valInt32_;
}

uint32_t PTPCamera::PropertyValue_t::getUint32(void) const
{
    assert(typeCode_ == TYPE_UINT32);
    return valUint32_;
}

int64_t PTPCamera::PropertyValue_t::getInt64(void) const
{
    assert(typeCode_ == TYPE_INT64);
    return valInt64_;
}

uint64_t PTPCamera::PropertyValue_t::getUint64(void) const
{
    assert(typeCode_ == TYPE_UINT64);
    return valUint64_;
}

INT128_t PTPCamera::PropertyValue_t::getInt128(void) const
{
    assert(typeCode_ == TYPE_INT128);
    return valInt128_;
}

UINT128_t PTPCamera::PropertyValue_t::getUint128(void) const
{
    assert(typeCode_ == TYPE_UINT128);
    return valUint128_;
}

string PTPCamera::PropertyValue_t::getString(void) const
{
    assert(typeCode_ == TYPE_STRING);
    return *valString_;
}

void PTPCamera::PropertyValue_t::setInt8(int8_t val)
{
    clear();
    typeCode_ = TYPE_INT8;
    valInt8_ = val;
}

void PTPCamera::PropertyValue_t::setUint8(uint8_t val)
{
    clear();
    typeCode_ = TYPE_UINT8;
    valUint8_ = val;
}

void PTPCamera::PropertyValue_t::setInt16(int16_t val)
{
    clear();
    typeCode_ = TYPE_INT16;
    valInt16_ = val;
}

void PTPCamera::PropertyValue_t::setUint16(uint16_t val)
{
    clear();
    typeCode_ = TYPE_UINT16;
    valUint16_ = val;
}

void PTPCamera::PropertyValue_t::setInt32(int32_t val)
{
    clear();
    typeCode_ = TYPE_INT32;
    valInt32_ = val;
}

void PTPCamera::PropertyValue_t::setUint32(uint32_t val)
{
    clear();
    typeCode_ = TYPE_UINT32;
    valUint32_ = val;
}

void PTPCamera::PropertyValue_t::setString(const string&val)
{
    clear();
    typeCode_ = TYPE_STRING;
    valString_ = new string(val);
}
