#include "driver.h"

BYTE PciRead8(UINT id, UINT reg)
{
    DWORD addr = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, addr);
    return __inbyte(PCI_CONFIG_DATA + (reg & 0x03));
}

WORD PciRead16(UINT id, UINT reg)
{
    DWORD addr = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, addr);
    return __inword(PCI_CONFIG_DATA + (reg & 0x02));
}

DWORD PciRead32(UINT id, UINT reg)
{
    DWORD addr = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, addr);
    return __indword(PCI_CONFIG_DATA);
}

VOID PciWrite8(UINT id, UINT reg, BYTE data)
{
    DWORD address = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, address);
    __outbyte(PCI_CONFIG_DATA + (reg & 0x03), data);
}

VOID PciWrite16(UINT id, UINT reg, WORD data)
{
    DWORD address = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, address);
    __outword(PCI_CONFIG_DATA + (reg & 0x02), data);
}

VOID PciWrite32(UINT id, UINT reg, DWORD data)
{
    DWORD address = 0x80000000 | id | (reg & 0xfc);
    __outdword(PCI_CONFIG_ADDR, address);
    __outdword(PCI_CONFIG_DATA, data);
}

static void PciReadBar(UINT id, UINT index, DWORD *address, DWORD *mask)
{
    UINT reg = PCI_CONFIG_BAR0 + index * sizeof(DWORD);

    *address = PciRead32(id, reg);

    PciWrite32(id, reg, 0xffffffff);
    *mask = PciRead32(id, reg);

    PciWrite32(id, reg, *address);
}

void PciGetBar(PciBar *bar, UINT id, UINT index)
{
    // Read pci bar register
    DWORD addressLow;
    DWORD maskLow;
    PciReadBar(id, index, &addressLow, &maskLow);

    if (addressLow & PCI_BAR_64)
    {
        // 64-bit mmio
        DWORD addressHigh;
        DWORD maskHigh;
        PciReadBar(id, index + 1, &addressHigh, &maskHigh);

        bar->u.address = (void *)(((uintptr_t)addressHigh << 32) | (addressLow & ~0xf));
        bar->size = ~(((QWORD)maskHigh << 32) | (maskLow & ~0xf)) + 1;
        bar->flags = addressLow & 0xf;
    } else if (addressLow & PCI_BAR_IO)
    {
        // i/o register
        bar->u.port = (WORD)(addressLow & ~0x3);
        bar->size = (WORD)(~(maskLow & ~0x3) + 1);
        bar->flags = addressLow & 0x3;
    } else
    {
        // 32-bit mmio
        bar->u.address = (void *)(uintptr_t)(addressLow & ~0xf);
        bar->size = ~(maskLow & ~0xf) + 1;
        bar->flags = addressLow & 0xf;
    }
}


