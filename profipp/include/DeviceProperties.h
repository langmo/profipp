#ifndef DEVICEPROPERTIES_H
#define DEVICEPROPERTIES_H

#pragma once


#include <string>
// header provides typedefs like uint32_t
#include <cstdint>
namespace profinet
{
    struct DeviceProperties
    {
        /** Device Specification **/
        /* GSDML tag: VendorID */
        uint16_t  vendorID{0x0493};

        /**
         *  GSDML Device configuration
         */
        // GSDML tag: VendorName
        std::string vendorName{"unknown vendor"};
        /* GSDML tag: DeviceID */
        uint16_t  deviceID{0x0002};
        std::string  deviceName{"unknown device"};
        // GDML tag: DeviceIdentity -> InfoText
        std::string deviceInfoText{"no device information available"};
        // GDML tag: DeviceFunction -> Family -> ProductFamily
        std::string deviceProductFamily{"general"};

        uint8_t numSlots{4};

        uint16_t  api{0};
        /**
         * @brief The profinet name of the station, which has to be supplied
         * together with the IP address and subnet mask
         */
        std::string stationName{"unknown-station"};        

        /* Used in DCP communication */
        uint16_t  oemVendorID{0xcafe};
        uint16_t  oemDeviceID{0xee02};

        /* Used in I&M0 */
        uint16_t  imHardwareRevision{3};
        uint16_t  imVersionMajor{1};
        uint16_t  imVersionMinor{2};

        // Current software version of controller of device.
        char swRevPrefix{'V'};/* Allowed: 'V', 'R', 'P', 'U', 'T' */
        uint8_t swRevMajor{0};
        uint8_t swRevMinor{2};
        uint8_t swRevPatch{0};

        // Current hardware version of device.
        char hwRevPrefix{'A'};
        uint8_t hwRevMajor{1};
        uint8_t hwRevMinor{0};

        uint16_t  profileID{0x1234};
        uint16_t  profileSpecType{0x5678};
        uint16_t  imRevisionCounter{0}; /* Typically 0 */

        /* Note: You need to read out the actual hardware serial number instead */
        std::string serialNumber{"007"};

        /* Initial values. Can be overwritten by PLC */
        std::string tagFunction{"my function"};
        std::string tagLocation{"my location"};
        std::string imDate{"2022-03-01 10:03"};
        std::string descriptor{"my descriptor"};
        std::string signature{""};

        /* GSDML tag: Writeable_IM_Records */
        static constexpr uint16_t supportedIM1{0x0002};
        static constexpr uint16_t supportedIM2{0x0004};
        static constexpr uint16_t supportedIM3{0x0008};
        static constexpr uint16_t supportedIM4{0x0010}; /** Should only be used together with functional safety */
        uint16_t  supportedIMs{supportedIM1 | supportedIM2 | supportedIM3};

        /* GSDML tag: OrderNumber */
        std::string orderID{"12345 Abcdefghijk"};

        /* GSDML tag: ModuleInfo / Name */
        std::string productName{"Unknown Application"};

        /* GSDML tag: MinDeviceInterval */
        uint16_t  minDeviceInterval{32}; /* 1 ms */

        uint16_t  diagCustomUSI{0x1234};

        /* See "Specification for GSDML" 8.26 LogBookEntryItem for allowed values */
        uint16_t  logbookErrorCode{0x20}; /* Manufacturer specific */
        uint16_t  logbookErrorDecode{0x82}; /* Manufacturer specific */
        uint16_t  logbookErrorCode1{0x4E};
        uint16_t  logbookErrorCode2{0x00};       /* Manufacturer specific */
        uint32_t  logbookEntryDetail{0xFEE1DEAD}; /* Manufacturer specific */

        uint16_t  defaultMautype{0x10}; /* Copper 100 Mbit/s Full duplex */
    };
}
#endif