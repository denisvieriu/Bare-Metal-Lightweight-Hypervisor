#include "pcci.h"
#include "map.h"
#include "minihv_status.h"
#include "string.h"
#include "print.h"
#include "ept.h"
#include "acpi.h"
#include "acpica.h"

#define MAX_NUMBER_OF_BUSES 256
#define MAX_NUMBER_OF_DEVICES 32
#define MAX_NUMBER_OF_FUNCTIONS 8

#define BUILD_CONFIG_ADDRESS(b, d, f, r) ((b << 16) | (d << 11) | (f << 8) | (r & 0xFC) | (0x1 << 31))

#define SERIAL_CONTROLLER_CLASS_CODE 0x07
#define SERIAL_CONTROLLER_SUBCLASS 0x00
#define SERIAL_CONTROLLER_PROG_IF 0x02

#define PCI_GET_SUBCLASS(x) (x & 0xFF)
#define PCI_GET_CLASS_CODE(x) ((x >> 8) & 0xFF)

static PCI_CONFIG_ADDRESS m_reservedSerialPortConfigAddresses = { 0 };
static QWORD m_reservedSerialPortEcamPa;

static
DWORD
PciConfigReadRegister(
    BYTE BusNumber,
    BYTE DeviceNumber,
    BYTE FunctionNumber,
    BYTE RegisterNumber
)
{
    DWORD configAddress = BUILD_CONFIG_ADDRESS(BusNumber, DeviceNumber, FunctionNumber, RegisterNumber);
    DWORD configData;

    __outdword(PCI_CONFIG_ADDRESS_PORT, configAddress);

    configData = __indword(PCI_CONFIG_DATA_PORT);

    return configData;
}

VOID
PciSearchDevice(
    WORD VendorId,
    WORD DeviceId,
    PWORD Port
)
{
    WORD tempVendorId;
    WORD tempDeviceId;
    BYTE classCode;
    BYTE subclass;
    BYTE progIf;

    for (DWORD i = 0; i < MAX_NUMBER_OF_BUSES; i++)
    {
        for (BYTE j = 0; j < MAX_NUMBER_OF_DEVICES; j++)
        {
            for (BYTE k = 0; k < MAX_NUMBER_OF_FUNCTIONS; k++)
            {
                DWORD firstRegister = PciConfigReadRegister((BYTE)i, j, k, 0x00);

                tempVendorId = (WORD)firstRegister;
                tempDeviceId = (WORD)((firstRegister >> 0x10) & 0xFFFF);

                if (tempVendorId == INVALID_VENDOR_ID || tempVendorId != VendorId || tempDeviceId != DeviceId)
                {
                    break;
                }

                DWORD thirdRegister = PciConfigReadRegister((BYTE)i, j, k, 0x08);

                classCode = (BYTE)((thirdRegister >> 24) & 0xFF);

                if (classCode != SERIAL_CONTROLLER_CLASS_CODE) continue;

                subclass = (BYTE)((thirdRegister >> 16) & 0xFF);

                if (subclass != SERIAL_CONTROLLER_SUBCLASS) continue;

                progIf = (BYTE)((thirdRegister >> 8) & 0xFF);

                if (progIf != SERIAL_CONTROLLER_PROG_IF) continue;

                // if we reach here, then we found our serial communication device
                if (*Port == 0)
                {
                    *Port = (WORD)(PciConfigReadRegister((BYTE)i, j, k, 0x10) & 0xFFFE);

                    m_reservedSerialPortConfigAddresses.FunctionNumber = k;
                    m_reservedSerialPortConfigAddresses.DeviceNumber = j;
                    m_reservedSerialPortConfigAddresses.BusNumber = i;
                }
            }
        }
    }
}

BOOLEAN
PciBDFEqualsReservedSerialPort(
    BYTE Bus,
    BYTE Device,
    BYTE Function
)
{
    return (m_reservedSerialPortConfigAddresses.BusNumber == Bus) && (m_reservedSerialPortConfigAddresses.DeviceNumber == Device) && (m_reservedSerialPortConfigAddresses.FunctionNumber == Function);
}



NTSTATUS
PciEcamRestrictAccessOnReservedSerialPort(
    VOID
)
{
    QWORD pciEcamBaseAddress;
    BYTE pciStartBusNumber;
    BYTE pciEndBusNumber;
    NTSTATUS status = STATUS_SUCCESS;
    QWORD reservedSerialPortEptEntryValue;

    status = AcpiGetPciEcam(&pciEcamBaseAddress, &pciStartBusNumber, &pciEndBusNumber);
    if (ACPI_FAILURE(status))
    {
        LOG("Acpit pci ecam failed");
        return status;
    }

    m_reservedSerialPortEcamPa = PciGetEcamFunctionPa(
        pciEcamBaseAddress,
        pciStartBusNumber,
        (BYTE)m_reservedSerialPortConfigAddresses.BusNumber,
        (BYTE)m_reservedSerialPortConfigAddresses.DeviceNumber,
        (BYTE)m_reservedSerialPortConfigAddresses.FunctionNumber
    );

    reservedSerialPortEptEntryValue = EptGetEntryValue(m_reservedSerialPortEcamPa);
    ((PEPT_PT_ENTRY)&reservedSerialPortEptEntryValue)->ReadAccess = 0;
    ((PEPT_PT_ENTRY)&reservedSerialPortEptEntryValue)->WriteAccess = 0;
    ((PEPT_PT_ENTRY)&reservedSerialPortEptEntryValue)->ExecuteAccessSupervisorMode = 0;

    EptSetEntryValue(m_reservedSerialPortEcamPa, reservedSerialPortEptEntryValue);

    return status;
}

QWORD
PciGetReservedSerialPortEcamPa(
    VOID
)
{
    return m_reservedSerialPortEcamPa;
}