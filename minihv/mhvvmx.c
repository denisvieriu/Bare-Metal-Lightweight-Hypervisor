#include "minihv.h"
#include "vmx.h"
#include "dbglog.h"
#include "cpu.h"
#include "minihv_status.h"
#include "cpuid.h"
#include "vmx_common.h"

DECLSPEC_NORETURN
VOID
MhvVmxResume(
    VOID
    )
{
    __vmx_vmresume();
}

QWORD
FORCEINLINE
MhvVmxRead(
    _In_ size_t Field
    )
{
    NTSTATUS    mHvStatus;
    VMX_STATUS  vmxStatus;
    size_t      fieldValue;  // It's address is passed to __vmx_read
                             // Here it's data will be kept

    vmxStatus = __vmx_vmread(Field, &fieldValue);

    //
    // Check if there was any error (convert it to a NTSTATUS)
    //
    mHvStatus = VmxStatusToNtStatus(vmxStatus);
    
    //
    // In case of error, halt everything (easier to debug)
    //
    if (!NT_SUCCESS(mHvStatus))
    {
        LOG_WARNING("MhvVmxRead failed, status: [0x%08x]", mHvStatus);
    }
    
    return fieldValue;
}

DWORD 
MhvVmxLaunch(
    VOID
    )
{ 
    //
    // Error numbers defined in INTEL MANUAL (Vol 3, Chapt. 30.4)
    //
    DWORD      vmErrorNumber;
    VMX_STATUS vmxStatus;
    NTSTATUS   mHvStatus;
    
    vmxStatus = __vmx_vmlaunch();
     
    //
    // This part should not be reached (only in case VMLAUNCH fails)
    //
    mHvStatus = VmxStatusToNtStatus(vmxStatus);
    LOG_WARNING("VMXLAUNCH failed, status: [0x%08x]", mHvStatus);

    vmErrorNumber = (DWORD)MhvVmxRead(VMCS_VM_INSTRUCTION_ERROR_ENCODING);
    LOG_WARNING("Error number retrieved (chapter 30.4): [%d]", vmErrorNumber);
    __vmx_off();

    //
    // Also return the error code back
    //
    return vmErrorNumber;
}

/*++

Reoutine Description:

    Check for VT-x suport

Return Value:

    TRUE  -  VT-x supported
             VMX supported
    FALSE -  otherwise

--*/
NTSTATUS
MhvVmxHardSupported(
    VOID
    )
{
    CPUID cpuInfo = { 0 };

    __cpuid((INT*)&cpuInfo, 0x01);

    //
    // Check if VMX bit is set (bit 5)
    //
    if (!IS_BIT_SET(cpuInfo.CpuidEcx, VMX))
    {
        return STATUS_VMX_NOT_SUPPORTED;
    }

    IA32_FEATURE_CONTROL_MSR control = { 0 };
    control.All = __readmsr(MSR_IA32_FEATURE_CONTROL);

    //
    // Check for bios lock
    //
    if (control.Fields.Lock == 0)
    {
        //
        // Set corresponding bits
        //
        control.Fields.Lock        = TRUE;
        control.Fields.EnableVmxon = TRUE;
        __writemsr(MSR_IA32_FEATURE_CONTROL, control.All);
    }
    else if (control.Fields.EnableVmxon == 0)
    {
        //
        // In this case, VMX features might be disabled from bios (nothing to do)
        //
        LOG_WARNING("VMX may be locked off in BIOS, please enable it!");
        return STATUS_VMX_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}

BOOLEAN 
MhvPresentHypervisor(
    VOID
    )
{
    CPUID cpuId = { 0 };
    __cpuid((INT*)&cpuId, 0x01);

    // Check if our MINI Hypervisor is there
    if (cpuId.CpuidEcx & HYPER_V_BIT)
    {
        // Now check for our signature, we should've set it in guest tries to access cpuid
        __cpuid((INT*)&cpuId, HYPER_V_INT);
        LOG_INFO("Check for our mhv (presence)");
        if (cpuId.CpuidEax == MHV_SIGN)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

NTSTATUS
VmxEntryRootOpAndCheckFeatures(
    _In_ PCPU Pcpu
    )
{
    //
    // Checks performed:
    //  -> True Msr support
    //  -> EPT Page walk = 4 (at most 4 supported tables)
    //  -> 
    //
    //
    // Check for capabilities requireded by MINI HV
    //


    LOG_INFO("Mini hypervisor to work requires (to be supported):\n"
        "\t EPT PAGE WALK OF LENGTH 4\n"
        "\t TRUE PINBASED CTRL\n");

    // A.2 (Volume 3)
    Pcpu->CapabReport.TrueMsrSupport         = FALSE;
    Pcpu->CapabReport.VmExitReportsInsOuts   = FALSE;
    Pcpu->CapabReport.EptPageWalkLengthFour  = FALSE;
    Pcpu->CapabReport.EptPgStructeWriteBack  = FALSE;
    Pcpu->CapabReport.EptPdeTwoMbPage        = FALSE;
    Pcpu->CapabReport.EptPdpteOneGbPage      = FALSE;
    Pcpu->CapabReport.VmExitAdvReport        = FALSE;
    Pcpu->CapabReport.DirtyFlagAccess        = FALSE;
    Pcpu->CapabReport.AccessRightsVmcsEnum   = FALSE;

    if (Pcpu->MsrData[0].QuadPart & (1ULL << 54))
    {
        Pcpu->CapabReport.VmExitReportsInsOuts = TRUE;
    }

    if (Pcpu->MsrData[0].QuadPart & (1ULL << 55))  // (VMX_TRUE_PIBASED_CTRL)
    {
        Pcpu->CapabReport.TrueMsrSupport = TRUE;
    }

    //
    // A.10 VPID AND EPT CAPABILITIES
    //
    if (Pcpu->MsrData[0xC].QuadPart & 1ULL)
    {
        Pcpu->CapabReport.AccessRightsVmcsEnum = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 6)) // EPT PAGE WALK OF LENGTH 4 supported
    {
        Pcpu->CapabReport.EptPageWalkLengthFour = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 14)) // logical processor allows EPT paging-structure memory to be writeback
    {
        Pcpu->CapabReport.EptPgStructeWriteBack = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 16))
    {
        Pcpu->CapabReport.EptPdeTwoMbPage = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 17))
    {
        Pcpu->CapabReport.EptPdpteOneGbPage = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 21))
    {
        Pcpu->CapabReport.DirtyFlagAccess = TRUE;
    }

    if (Pcpu->MsrData[0xC].QuadPart & (1ULL << 22))
    {
        Pcpu->CapabReport.VmExitAdvReport = TRUE;
    }

    //
    // A.11 VM functions
    //
    if (Pcpu->MsrData[0x11].QuadPart & 0xFFFF)
    {

    }



    LOG_INFO("Information retrieved about compatibility:\n"
        "TrueMsrSupport         = %d\n"
        "VmExitReportsInsOuts   = %d\n"
        "EptPageWalkLengthFour  = %d\n"
        "EptPgStructeWriteBack  = %d\n"
        "EptPdeTwoMbPage        = %d\n"
        "EptPdpteOneGbPage      = %d\n"
        "VmExitAdvReport        = %d\n"
        "DirtyFlagAccess        = %d\n",
        Pcpu->CapabReport.TrueMsrSupport,
        Pcpu->CapabReport.VmExitReportsInsOuts,
        Pcpu->CapabReport.EptPageWalkLengthFour,
        Pcpu->CapabReport.EptPgStructeWriteBack,
        Pcpu->CapabReport.EptPdeTwoMbPage,
        Pcpu->CapabReport.EptPdpteOneGbPage,
        Pcpu->CapabReport.VmExitAdvReport,
        Pcpu->CapabReport.DirtyFlagAccess);
    if (Pcpu->CapabReport.EptPageWalkLengthFour == FALSE)
    {
        //
        // Page walk of length 4 not supported
        //
        return STATUS_VMX_EPT_PAGE_WALK_FOUR_NOT_SUPP;
    }

    return !(Pcpu->CapabReport.EptPageWalkLengthFour ||
        Pcpu->CapabReport.TrueMsrSupport             ||
        Pcpu->CapabReport.VmExitAdvReport            ||
        Pcpu->CapabReport.VmExitReportsInsOuts       ||
        Pcpu->CapabReport.EptPdeTwoMbPage            ||
        Pcpu->CapabReport.DirtyFlagAccess)
        ? (STATUS_VMX_NOT_SUPPORTED) : (STATUS_SUCCESS);
}

//
NTSTATUS 
ControlsInit(
    _In_ PCPU Pcpu
    )
{

    UNREFERENCED_PARAMETER(Pcpu);
    // all seq. of __vmx_vmwrite here
    // Reach here only if pinbased controls are validated

    QWORD pinbaseCtrl;
    QWORD primCtrl;

    pinbaseCtrl = 0;
    primCtrl = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
VmxVmLaunch(
    VOID
    )
{
    DWORD    error; 

    error = MhvVmxLaunch();

    //
    // Reaching here only if _vmx_vmlaunch fails
    // 

    LOG_ERROR("[ERROR] VmxVmlauch: [%d]", error);
    return STATUS_VMX_LAUNCH_FAILED;
}

NTSTATUS
MhvVmxInit(
    _In_ PCPU pCpu
    )
{
    INT idx;
    NTSTATUS mHvStatus;

    //
    // Initialize VMX-related structures
    // 
    for (idx = 0; idx < _countof(pCpu->MsrData); idx++)
    {
        //
        // Virtual processor (Appendix A VMX Capability Reporting Facility)
        //
        pCpu->MsrData[idx].QuadPart = __readmsr(MSR_IA32_VMX_BASIC + idx);
    }

    mHvStatus = MhvVmxHardSupported();
    if (!NT_SUCCESS(mHvStatus))
    {
        LOG_WARNING("Hard not supported! Error code: [0x%08x]");
    }

    mHvStatus = VmxEntryRootOpAndCheckFeatures(pCpu);
    if (!NT_SUCCESS(mHvStatus))
    {
        LOG_WARNING("Controls requiered by Mini Hypervisor are not supported on this machine! Error code: [0x%08x]")
    }

    mHvStatus = ControlsInit(pCpu);
    if (!NT_SUCCESS(mHvStatus))
    {
        LOG_WARNING("Failed To initialize features controles, status: [0x%08x]", mHvStatus);
    }

    mHvStatus = VmxVmLaunch();
    if (!NT_SUCCESS(mHvStatus))
    {
        LOG_WARNING("VmLaunch failed, status code: [0x%08x]", mHvStatus);

    }


    
    LOG_ERROR("Going to HALT, nothing left to do");
    __halt();


    // VM SUPPORT ENTRY ROOT
    // VMLAUNCH

    // VMLAUNCH SHOULD NOT REACH HERE

    //__halt();
    return TRUE;

}



