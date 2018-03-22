#pragma once

#include "minihv.h"
#include "minihv_status.h"

#define PCI_CONFIG_ADDRESS_PORT 0xCF8
#define PCI_CONFIG_DATA_PORT 0xCFC

#define INVALID_VENDOR_ID 0xFFFF

#define PCI_CONFIG_ADDRESS_FUNCTION_SHIFT 0x08
#define PCI_CONFIG_ADDRESS_DEVICE_SHIFT 0x0B
#define PCI_CONFIG_ADDRESS_BUS_SHIFT 0x10

#define PCI_CONFIG_ADDRESS_FUNCTION_MASK 0x7
#define PCI_CONFIG_ADDRESS_DEVICE_MASK 0x1F
#define PCI_CONFIG_ADDRESS_BUS_MASK 0xFF

typedef struct _PCI_CONFIG_ADDRESS
{
    DWORD Zero : 2;
    DWORD RegisterNumber : 6;
    DWORD FunctionNumber : 3;
    DWORD DeviceNumber : 5;
    DWORD BusNumber : 8;
    DWORD Reserved : 7;
    DWORD EnableBit : 1;
} PCI_CONFIG_ADDRESS, *PPCI_CONFIG_ADDRESS;

BOOLEAN
PciBDFEqualsReservedSerialPort(
    BYTE Bus,
    BYTE Device,
    BYTE Function
);

__forceinline
QWORD
PciGetEcamFunctionPa(
    QWORD BaseAddress,
    BYTE StartBus,
    BYTE Bus,
    BYTE Device,
    BYTE Function
)
{
    return BaseAddress + (((Bus - StartBus) << 20) | (Device << 15) | (Function << 12));
}

NTSTATUS
PciEcamRestrictAccessOnReservedSerialPort(
    VOID
);

VOID
PciSearchDevice(
    WORD VendorId,
    WORD DeviceId,
    PWORD Port
);

QWORD
PciGetReservedSerialPortEcamPa(
    VOID
);
