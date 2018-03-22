#include "minihv.h"
#include "pci.h"
#include "registry.h"
#include "driver.h"
#include "print.h"
#include "crt.h"
#include "pcci.h"

#define MOSCHIP_DEVICE_ID   0x9912
#define MOSCHIP_VENDOR_ID   0x9710

DWORD gPort;

static WORD m_reservedSerialPort = 0;

void InitSerial(UINT id);

static VOID
PciVisit(
    UINT bus,
    UINT dev,
    UINT func
)
{
    UINT id = PCI_MAKE_ID(bus, dev, func);

    PCI_DEVICE_INFO info;
    info.VendorId = PciRead16(id, PCI_CONFIG_VENDOR_ID);
    if (info.VendorId == 0xFFFF)
    {
        return;
    }

    info.DeviceId = PciRead16(id, PCI_CONFIG_DEVICE_ID);
    info.ProgIntf = PciRead8(id, PCI_CONFIG_PROG_INTF);
    info.Subclass = PciRead8(id, PCI_CONFIG_SUBCLASS);
    info.ClassCode = PciRead8(id, PCI_CONFIG_CLASS_CODE);


   /* LOG_PCI("%02x:%02x:%d 0x%04x/0x%04x: %s\n",
        bus, dev, func,
        info.VendorId, info.DeviceId,
        PciClassName(info.ClassCode, info.Subclass, info.ProgIntf)
    );*/

    if (info.DeviceId == MOSCHIP_DEVICE_ID && info.VendorId == MOSCHIP_VENDOR_ID && gPort == 0)
    {
        PciSearchDevice(MOSCHIP_VENDOR_ID, MOSCHIP_DEVICE_ID, &m_reservedSerialPort);
        InitSerial(id);
    }
}



VOID
InitSerial(
    UINT id
)
{
    PciBar bar;
    PciGetBar(&bar, id, 0);
    gPort = bar.u.port;

    if ((bar.flags & PCI_BAR_IO) == 0)
    {
        gPort = 0;
        return;
    }


    __outbyte(gPort + 1, 0x00);      // Disable all intrerrupts
    __outbyte(gPort + 3, 0x80);      // Enable DLAB (set baud rate divisor)
    __outbyte(gPort + 0, 0x01);      // Set divisor to 1 (lo byte) 115200 baud rate
    __outbyte(gPort + 1, 0x00);      //                  (hi byte)
    __outbyte(gPort + 3, 0x03);      // 8 bits, no parity, one stop bit
    __outbyte(gPort + 2, 0xC7);      // Enable FIFO, clear them, with 14-byte threshold
    __outbyte(gPort + 4, 0x0B);      // IRQs enabled, RTS/DSR set
}

INT
isTransmitEmpty(
    VOID)
{

    // this should be called in case of empty
    return __inbyte(gPort + 5) & 0x20;
}

BOOLEAN
SerialPut(
    CHAR chr
)
{
    while (isTransmitEmpty() == 0);
    __outbyte(gPort, chr);
    return TRUE;
}

#pragma warning (disable : 4090)
BOOLEAN
SerialWrite(
    CONST CHAR *fmt, ...
)
{
    CHAR buf[1024];
    va_list args;

    va_start(args, fmt);
    CustomSnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    CHAR c;
    CHAR *s = buf;

    while ((c = *s++))
    {
        SerialPut(c);
    }
    return TRUE;
}


VOID PciInit(
    VOID
)
{
    for (UINT bus = 0; bus < 256; bus++)
    {
        for (UINT dev = 0; dev < 32; dev++)
        {
            UINT baseId = PCI_MAKE_ID(bus, dev, 0);
            BYTE headerType = PciRead8(baseId, PCI_CONFIG_HEADER_TYPE);
            UINT funcCount = headerType & PCI_TYPE_MULTIFUNC ? 8 : 1;

            for (UINT func = 0; func < funcCount; func++)
            {
                PciVisit(bus, dev, func);
            }
        }
    }
}