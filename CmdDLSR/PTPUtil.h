/**
 * PTPUtil.h -- PTP utilities.
 *
 * Copyright (c) Leon (yong.he1982(at)gmail.com)
 * Date  : 2013-08-07 [Wednesday, 219]
 */

#ifndef _HEADER_PTPUTIL_H_
#define _HEADER_PTPUTIL_H_

#include <inttypes.h>
#include <string>

/**
 * Return the string form of the opcode.
 * If it is a vendor specific code, the vendorId is used.
 */
extern std::string ptpOpcodeString(uint16_t code, uint32_t vendorId);

/**
 * Return the string form of the event code.
 * If it is a vneder specific code, the vendorId is used.
 */
extern std::string ptpEventString(uint16_t code, uint32_t vendorDd);

/**
 * Return the string form of the property code.
 * If it is a vneder specific code, the vendorId is used.
 */
extern std::string ptpPropertyString(uint16_t code, uint32_t vendorId);

/**
 * Return the string form of the property value.
 * If it is a vneder specific code, the vendorId is used.
 */
extern std::string ptpPropertyUint8String(uint16_t code, uint8_t value,
                                          uint32_t vendorId);
extern std::string ptpPropertyUint16String(uint16_t code, uint16_t value,
                                           uint32_t vendorId);
extern std::string ptpPropertyUint32String(uint16_t code, uint32_t value,
                                           uint32_t vendorId);

#endif /* ~_HEADER_PTPUTIL_H_ */
