/**
 * PTPCamera.h -- PTP protocal camera abstraction.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#ifndef _HEADER_PTPCAMERA_H_
#define _HEADER_PTPCAMERA_H_

#include <inttypes.h>
#include <vector>
#include <map>
#include <ostream>
#include <string>

class INT128_t
{
    public:
        bool operator == (const INT128_t &that) const {
            return high_ == that.high_ && low_ == that.low_;
        }

    private:
        uint64_t high_;
        uint64_t low_;
};

class UINT128_t
{
    public:
        bool operator == (const UINT128_t &that) const {
            return high_ == that.high_ && low_ == that.low_;
        }

    private:
        uint64_t high_;
        uint64_t low_;
};

class PTPCamera
{
    public:
        enum TypeCode_t {
            TYPE_NONE   = 0x0000,
            TYPE_INT8   = 0x0001,
            TYPE_UINT8  = 0x0002,
            TYPE_INT16  = 0x0003,
            TYPE_UINT16 = 0x0004,
            TYPE_INT32  = 0x0005,
            TYPE_UINT32 = 0x0006,
            TYPE_INT64  = 0x0007,
            TYPE_UINT64 = 0x0008,
            TYPE_INT128 = 0x0009,
            TYPE_UINT128 = 0x000a,
            TYPE_STRING = 0xffff
        };

        enum TypeForm_t {
            FORM_NONE  = 0,
            FORM_RANGE = 1,
            FORM_ENUM  = 2
        };

        enum PropCode_t {
            PROP_NONE                = 0x0000,
            PROP_ImageSize           = 0x5003,
            PROP_WhiteBalance        = 0x5005,
            PROP_FNumber             = 0x5007,
            PROP_FocusMode           = 0x500a,
            PROP_FlashMode           = 0x500c,
            PROP_ExposureTime        = 0x500d,
            PROP_ExposureProgramMode = 0x500e,
            PROP_ExposureIndex       = 0x500f
        };

        // PTP property value types wrapper.
        class PropertyValue_t
        {
            public:
                PropertyValue_t();
                ~PropertyValue_t();

                PropertyValue_t(const PropertyValue_t &);

                PropertyValue_t& operator = (const PropertyValue_t &);
                bool operator == (const PropertyValue_t &that) const;
                bool operator != (const PropertyValue_t &that) const {
                    return !(*this == that);
                }

                // Clear property value.
                void clear();
                void setInt8(int8_t);
                void setUint8(uint8_t);
                void setInt16(int16_t);
                void setUint16(uint16_t);
                void setInt32 (int32_t);
                void setUint32(uint32_t);
                // intended the transform from NSString to std::string at caller side
                // TODO: use NSString?
                void setString(const std::string&);

                // Get the last set type code.
                TypeCode_t getType() const {
                    return typeCode_;
                }

                int8_t   getInt8  () const;
                uint8_t  getUint8 () const;
                int16_t  getInt16 () const;
                uint16_t getUint16() const;
                int32_t  getInt32 () const;
                uint32_t getUint32() const;
                int64_t  getInt64 () const;
                uint64_t getUint64() const;
                INT128_t  getInt128 () const;
                UINT128_t getUint128() const;
                std::string  getString() const;

            private:
                PTPCamera::TypeCode_t typeCode_ : 16;

                union {
                    int8_t valInt8_;
                    uint8_t valUint8_;
                    int16_t valInt16_;
                    uint16_t valUint16_;
                    int32_t valInt32_;
                    uint32_t valUint32_;
                    int64_t valInt64_;
                    uint64_t valUint64_;
                    INT128_t valInt128_;
                    UINT128_t valUint128_;
                    std::string* valString_;
                };

                void copy_(const PropertyValue_t& that);
        };

        static const PropertyValue_t s_propertyValueNil;

        static std::string resultCode(uint32_t resultCode);

    public:
        PTPCamera();
        virtual ~PTPCamera() = 0;

        // Standard PTP operation.
        // Return:
        //   true: the operation is supported, resultCode is set from device.
        //  false: the operation is not supported, resultCode is untouched.
        bool ptpInitiateCapture(uint32_t& resultCode);

        // Get device information from the camera by sending
        // a command to the device and processing the response.
        uint32_t ptpGetDeviceInfo(void);

        // Get PTP standard version that the camera supports.
        uint16_t ptpGetStandardVersion() const;

        // Get the camera vendor's extension information.
        uint32_t ptpGetExtensionVendor(bool rawId = false) const;
        uint16_t ptpGetExtensionVersion() const;
        std::string ptpGetExtensionDescription() const;

        bool ptpIsOperationCodeSupported(uint16_t code) const;

        // Some standard PTP properties
        uint16_t ptpFunctionalMode() const;
        std::string ptpManufacturer() const;
        std::string ptpModel() const;
        std::string ptpDeviceVersion() const;
        std::string ptpSerialNumber() const;

        int ptpBatteryLevel();

        // Map a PTP code to a descriptive string. The PTP code may be
        // any of a variety of codes, and the std::string is a human
        // readable description.
        typedef std::pair<uint16_t, std::string> CodeStringPair_t;

        // Return a list of supported operations, by name.
        // (Used for debugging or display purposes.)
        std::vector<std::string> ptpGetOperationsList() const;
        std::vector<std::string> ptpGetEventsList() const;
        std::vector<CodeStringPair_t> ptpGetPropertiesList() const;
        std::vector<std::string> ptpGetCaptureFormatsList() const;
        std::vector<std::string> ptpGetImageFormatsList() const;

        // Use the GetDevicePropDesc command to get all the property details, and
        // refresh the current and factory values.
        void ptpProbeProperty(unsigned propCode, uint32_t& resultCode);

        TypeCode_t ptpGetPropertyTypeCode(unsigned propCode) const;
        TypeForm_t ptpGetPropertyTypeForm(unsigned propCode) const;
        bool ptpIsPropertySetable(unsigned propCode) const;

        // Get the ordered list of enumerated property values, and
        // return the current value as an index. Return -1 if this is
        // not an enumerated type.
        struct LabeledValue_t {
            std::string label;
            PropertyValue_t value;
        };

        int ptpGetPropertyEnum(unsigned propCode, std::vector<LabeledValue_t>& table) const;

        // Get the range for the type and return true.
        // If this is not a range type then return false.
        bool ptpGetPropertyIsRange(unsigned propCode, PropertyValue_t& min, PropertyValue_t& max, PropertyValue_t& step) const;

        // Get the property current or factory default values. The values are cached
        // from the last ptpProbeProperty().
        const PropertyValue_t& ptpGetPropertyCurrent(unsigned propCode) const;
        const PropertyValue_t& ptpGetPropertyFactory(unsigned propCode) const;

        // Get the property directly from the camera, and save the result as the
        // current value for the property. This command goes directly to the camera.
        const PropertyValue_t& ptpGetCameraProperty(unsigned propCode, uint32_t& resultCode);

        // Set the property. The type of the property must match the type that the
        // camera expects.
        void ptpSetProperty(unsigned propCode, const PropertyValue_t& propVal, uint32_t& resultCode);

    protected:
        // The derived class implements this method to pass commands
        // (and responses) between this instance and the actual device.
        virtual uint32_t ptpCommand(uint16_t command, const std::vector<uint32_t>& parms,
                                    const unsigned char* send, size_t sizeSend,
                                    unsigned char* recv, size_t sizeRecv) = 0;

    private:
        uint16_t ptpStandardVersion_;
        uint32_t vendorExtensionId_;
        uint16_t vendorExtensionVersion_;
        std::string  vendorExtensionDescription_;
        uint16_t functionalMode_;
        std::vector<uint16_t> operationsSupported_;
        std::vector<uint16_t> eventsSupported_;
        std::vector<uint16_t> captureFormats_;
        std::vector<uint16_t> imageFormats_;
        std::string deviceManufacturer_;
        std::string deviceModel_;
        std::string deviceVersion_;
        std::string serialNumber_;

        struct PropertyInfo_t {
            PTPCamera::TypeCode_t typeCode : 16;
            // 1: get, 2: set
            uint8_t  getSetFlag;
            // 1: range, 2: enum
            uint8_t  formFlag;
            // Describe the factory default value for this property
            PropertyValue_t factory;
            // The current value for this value
            PropertyValue_t current;
            // The range is the list of possible values if this is an enum, or is
            // the range in 3 values: min, max, step.
            std::vector<PropertyValue_t> range;
        };

        // A map of supported properties with descriptions for the value.
        std::map<uint16_t, PropertyInfo_t> devicePropertiesSupportedMap_;

        static std::map<uint16_t, TypeCode_t> s_PtpTypeToTypeCode_;

        // Get (raw) the current value of the given property directly
        // from the camera.
        uint8_t  ptpGetPropertyUint8_(unsigned propCode, uint32_t& resultCode);
        uint16_t ptpGetPropertyUint16_(unsigned propCode, uint32_t& resultCode);
        uint32_t ptpGetPropertyUint32_(unsigned propCode, uint32_t& resultCode);
        int8_t  ptpGetPropertyInt8_(unsigned propCode, uint32_t& resultCode);
        int16_t ptpGetPropertyInt16_(unsigned propCode, uint32_t& resultCode);
        int32_t ptpGetPropertyInt32_(unsigned propCode, uint32_t& resultCode);
        std::string ptpGetPropertyString_(unsigned propCode, uint32_t& resultCode);

        // Set the value to the given property.
        void ptpSetPropertyUint8_(unsigned propCode, uint8_t val, uint32_t& resultCode);
        void ptpSetPropertyUint16_(unsigned propCode, uint16_t val, uint32_t& resultCode);
        void ptpSetPropertyUint32_(unsigned propCode, uint32_t val, uint32_t& resultCode);
        void ptpSetPropertyInt8_(unsigned propCode, int8_t val, uint32_t& resultCode);
        void ptpSetPropertyInt16_(unsigned propCode, int16_t val, uint32_t& resultCode);
        void ptpSetPropertyInt32_(unsigned propCode, int32_t val, uint32_t& resultCode);
        void ptpSetPropertyString_(unsigned propCode, const std::string& val, uint32_t& resultCode);

    private:
        // Copy constructor not supported
        PTPCamera(const PTPCamera &);
        PTPCamera& operator = (const PTPCamera &);
};

extern std::ostream& operator << (std::ostream & out, const PTPCamera::PropertyValue_t & val);

#endif /* ~_HEADER_PTPCAMERA_H_ */
