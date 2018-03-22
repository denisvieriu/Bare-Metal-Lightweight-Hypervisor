#include "vmx.h"
#include "local_apic.h"
#include "dbglog.h"
#include "cpuid.h"
#include "map.h"
#include "minihv_status.h"
#include "memzero.h"
#include "gdt.h"
#include "cpu.h"
#include "pcci.h"
#include "distorm.h"
#include "console.h"
#include "vmx_exit_handler.h"

#define IMPLEMENTED     TRUE
#define NOT_IMPLEMENTED !IMPLEMENTED
#define VMCS_LOGS       TRUE
//#pragma comment(lib, "distorm.lib")

#define SIZE_DWORD 32
#define MAX_SIZE PAGE_SIZE

#define CR4_VMXE_BIT_ENABLE         13

#define MSR_IA32_FEATURE_CONTROL    0x3A

// VMX-FIXED BITS in CR0 (A.7)
#define MSR_IA32_VMX_CR0_FIXED0     0x486
#define MSR_IA32_VMX_CR0_FIXED1     0x487

// VMX-FIXED BITS in CR4 (A.8)
#define MSR_IA32_VMX_CR4_FIXED0     0x488
#define MSR_IA32_VMX_CR4_FIXED1     0x489

// BASIC VMX OPERATION
#define MSR_IA32_VMX_BASIC              0x480

#define MSR_IA32_VMX_PINBASED_CTLS      0x481
#define MSR_IA32_VMX_PROCBASED_CTLS     0x482
#define MSR_IA32_VMX_PROCBASED_CTLS2    0x48B
#define MSR_IA32_VMX_EXIT_CTLS_MSR      0x483
#define MSR_IA32_VMX_ENTRY_CTLS_MSR     0x484
#define MSR_IA32_VMX_MISC               0x485

#define ACTIVATE_PREEMPTION_TIMER 0
#define ACTIVATE_HLT_EXITING 0


// Used for SSE
#define CR0_EM          0x02
#define CR0_MP          0x01
#define CR4_OSFXSR      0x09
#define CR4_OSXMMEXCPT  0x0A

//#define VMX_TO_STRING(Vmx, Val)                                                                                                                                                         \
//{                                                                                                                                                                                       \
//    do                                                                                                                                                                                  \
//    {                                                                                                                                                                                   \
//        SetGlobalVal(Val);                                                                                                                                                              \
//        SetNewLine(FALSE);                                                                                                                                                              \
//        switch (Vmx)                                                                                                                                                                    \
//        {                                                                                                                                                                               \
//            /* 16-Bit Control Fields     (value of --> 0 <-- in bits 14:13 indicates a 64-bit field) */                                                                                 \
//                                                                                                                                                                                        \
//            /* 16-Bit Control Fields     (value of --> 0 <-- in bits 11:10 indicates a control field) */                                                                                \
//        case VMCS_VPID_ENCODING:                                        VMX_SHORT_LOG("VMCS_VPID_ENCODING");                                                                            \
//        case VMCS_POSTED_INTERRUPT_NOTIFICATION_VECTOR_ENCODING:        VMX_SHORT_LOG("VMCS_POSTED_INTERRUPT_NOTIFICATION_VECTOR_ENCODING");                                            \
//        case VMCS_EPTP_INDEX_ENCODING:                                  VMX_SHORT_LOG("VMCS_EPTP_INDEX_ENCODING");                                                                      \
//                                                                                                                                                                                        \
//            /* 16-Bit Guest-State Fields (value of --> 2 <-- in bits 11:10 indicates a guest-state area) */                                                                             \
//        case VMCS_GUEST_ES_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_ES_ENCODING");                                                                        \
//        case VMCS_GUEST_CS_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_CS_ENCODING");                                                                        \
//        case VMCS_GUEST_SS_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_SS_ENCODING");                                                                        \
//        case VMCS_GUEST_DS_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_DS_ENCODING");                                                                        \
//        case VMCS_GUEST_FS_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_FS_ENCODING");                                                                        \
//        case VMCS_GUEST_GS_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_GS_ENCODING");                                                                        \
//        case VMCS_GUEST_LDTR_ENCODING:                                  VMX_SHORT_LOG("VMCS_GUEST_LDTR_ENCODING");                                                                      \
//        case VMCS_GUEST_TR_ENCODING:                                    VMX_SHORT_LOG("VMCS_GUEST_TR_ENCODING");                                                                        \
//        case VMCS_GUEST_INTERRUPT_STATUS_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_INTERRUPT_STATUS_ENCODING");                                                          \
//        case VMCS_GUEST_PML_ENCODING:                                   VMX_SHORT_LOG("VMCS_GUEST_PML_ENCODING");                                                                       \
//                                                                                                                                                                                        \
//            /* 16-Bit Host-State Fields  (value of --> 3 <-- in bits 11:10 indicates a host-state area) */                                                                              \
//        case VMCS_HOST_ES_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_ES_ENCODING");                                                                         \
//        case VMCS_HOST_CS_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_CS_ENCODING");                                                                         \
//        case VMCS_HOST_SS_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_SS_ENCODING");                                                                         \
//        case VMCS_HOST_DS_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_DS_ENCODING");                                                                         \
//        case VMCS_HOST_FS_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_FS_ENCODING");                                                                         \
//        case VMCS_HOST_GS_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_GS_ENCODING");                                                                         \
//        case VMCS_HOST_TR_ENCODING:                                     VMX_SHORT_LOG("VMCS_HOST_TR_ENCODING");                                                                         \
//                                                                                                                                                                                        \
//            /* 64-Bit Control Fields     (value of --> 1 <-- in bits 14:13 indicates a 64-bit field) */                                                                                 \
//                                                                                                                                                                                        \
//            /* 64-Bit Control Fields     (value of --> 0 <-- in bits 11:10 indicates a control field) */                                                                                \
//        case VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING");                                             \
//        case VMCS_ADDRESS_OF_IO_BITMAP_B_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_ADDRESS_OF_IO_BITMAP_B_FULL_ENCODING");                                             \
//        case VMCS_ADDRESS_OF_MSR_BITMAPS_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_ADDRESS_OF_MSR_BITMAPS_FULL_ENCODING");                                             \
//        case VMCS_VM_EXIT_MSR_STORE_ADDRESS_FULL_ENCODING:                      VMX_SHORT_LOG("VMCS_VM_EXIT_MSR_STORE_ADDRESS_FULL_ENCODING");                                          \
//        case VMCS_VM_EXIT_MSR_LOAD_ADDRESS_FULL_ENCODING:                       VMX_SHORT_LOG("VMCS_VM_EXIT_MSR_LOAD_ADDRESS_FULL_ENCODING");                                           \
//        case VMCS_VM_ENTRY_MSR_LOAD_ADDRESS_FULL_ENCODING:                      VMX_SHORT_LOG("VMCS_VM_ENTRY_MSR_LOAD_ADDRESS_FULL_ENCODING");                                          \
//        case VMCS_EXECUTIVE_VMCS_POINTER_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_EXECUTIVE_VMCS_POINTER_FULL_ENCODING");                                             \
//        case VMCS_PML_ADDRESS_FULL_ENCODING:                                    VMX_SHORT_LOG("VMCS_PML_ADDRESS_FULL_ENCODING");                                                        \
//        case VMCS_TSC_OFFSET_FULL_ENCODING:                                     VMX_SHORT_LOG("VMCS_TSC_OFFSET_FULL_ENCODING");                                                         \
//        case VMCS_VIRTUAL_APIC_ADDRESS_FULL_ENCODING:                           VMX_SHORT_LOG("VMCS_VIRTUAL_APIC_ADDRESS_FULL_ENCODING");                                               \
//        case VMCS_APIC_ACCESS_ADDRESS_FULL_ENCODING:                            VMX_SHORT_LOG("VMCS_APIC_ACCESS_ADDRESS_FULL_ENCODING");                                                \
//        case VMCS_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS_FULL_ENCODING:            VMX_SHORT_LOG("VMCS_POSTED_INTERRUPT_DESCRIPTOR_ADDRESS_FULL_ENCODING");                                \
//        case VMCS_VM_FUNCTION_CONTROLS_FULL_ENCODING:                           VMX_SHORT_LOG("VMCS_VM_FUNCTION_CONTROLS_FULL_ENCODING");                                               \
//        case VMCS_EPT_POINTER_FULL_ENCODING:                                    VMX_SHORT_LOG("VMCS_EPT_POINTER_FULL_ENCODING");                                                        \
//        case VMCS_EOI_EXIT_BITMAP_0_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_EOI_EXIT_BITMAP_0_FULL_ENCODING");                                                  \
//        case VMCS_EOI_EXIT_BITMAP_1_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_EOI_EXIT_BITMAP_1_FULL_ENCODING");                                                  \
//        case VMCS_EOI_EXIT_BITMAP_2_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_EOI_EXIT_BITMAP_2_FULL_ENCODING");                                                  \
//        case VMCS_EOI_EXIT_BITMAP_3_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_EOI_EXIT_BITMAP_3_FULL_ENCODING");                                                  \
//        case VMCS_EPTP_LIST_ADDRESS_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_EPTP_LIST_ADDRESS_FULL_ENCODING");                                                  \
//        case VMCS_VMREAD_BITMAP_ADDRESS_FULL_ENCODING:                          VMX_SHORT_LOG("VMCS_VMREAD_BITMAP_ADDRESS_FULL_ENCODING");                                              \
//        case VMCS_VMWRITE_BITMAP_ADDRESS_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_VMWRITE_BITMAP_ADDRESS_FULL_ENCODING");                                             \
//        case VMCS_VIRTUALIZATION_EXCEPTION_INFORMATION_ADDRESS_FULL_ENCODING:   VMX_SHORT_LOG("VMCS_VIRTUALIZATION_EXCEPTION_INFORMATION_ADDRESS_FULL_ENCODING");                       \
//        case VMCS_XSS_EXITING_BITMAP_FULL_ENCODING:                             VMX_SHORT_LOG("VMCS_XSS_EXITING_BITMAP_FULL_ENCODING");                                                 \
//        case VMCS_ENCLS_EXITING_BITMAP_FULL_ENCODING:                           VMX_SHORT_LOG("VMCS_ENCLS_EXITING_BITMAP_FULL_ENCODING");                                               \
//        case VMCS_TSC_MULTIPLIER_FULL_ENCODING:                                 VMX_SHORT_LOG("VMCS_TSC_MULTIPLIER_FULL_ENCODING");                                                     \
//                                                                                                                                                                                        \
//            /* 64-Bit Read-Only Data Field  (value of --> 1 <-- in bits 11:10 indicates a read-only data field) */                                                                      \
//        case VMCS_GUEST_PHYSICAL_ADDRESS_FULL_ENCODING:                         VMX_SHORT_LOG("VMCS_GUEST_PHYSICAL_ADDRESS_FULL_ENCODING");                                             \
//                                                                                                                                                                                        \
//            /* 64-Bit Guest-State Feilds    (value of --> 2 <-- in bits 11:10 indicates a guest-state area) */                                                                          \
//        case VMCS_LINK_POINTER_FULL_ENCODING:                                   VMX_SHORT_LOG("VMCS_LINK_POINTER_FULL_ENCODING");                                                       \
//        case VMCS_GUEST_IA32_DEBUGCTL_FULL_ENCODING:                            VMX_SHORT_LOG("VMCS_GUEST_IA32_DEBUGCTL_FULL_ENCODING");                                                \
//        case VMCS_GUEST_IA32_PAT_FULL_ENCODING:                                 VMX_SHORT_LOG("VMCS_GUEST_IA32_PAT_FULL_ENCODING");                                                     \
//        case VMCS_GUEST_IA32_EFER_FULL_ENCODING:                                VMX_SHORT_LOG("VMCS_GUEST_IA32_EFER_FULL_ENCODING");                                                    \
//        case VMCS_GUEST_IA32_PERF_GLOBAL_CTRL_FULL_ENCODING:                    VMX_SHORT_LOG("VMCS_GUEST_IA32_PERF_GLOBAL_CTRL_FULL_ENCODING");                                        \
//        case VMCS_GUEST_PDEPT0_FULL_ENCODING:                                   VMX_SHORT_LOG("VMCS_GUEST_PDEPT0_FULL_ENCODING");                                                       \
//        case VMCS_GUEST_PDEPT1_FULL_ENCODING:                                   VMX_SHORT_LOG("VMCS_GUEST_PDEPT1_FULL_ENCODING");                                                       \
//        case VMCS_GUEST_PDEPT2_FULL_ENCODING:                                   VMX_SHORT_LOG("VMCS_GUEST_PDEPT2_FULL_ENCODING");                                                       \
//        case VMCS_GUEST_PDEPT3_FULL_ENCODING:                                   VMX_SHORT_LOG("VMCS_GUEST_PDEPT3_FULL_ENCODING");                                                       \
//        case VMCS_GUEST_IA32_BNDCFGS_FULL_ENCODING:                             VMX_SHORT_LOG("VMCS_GUEST_IA32_BNDCFGS_FULL_ENCODING");                                                 \
//                                                                                                                                                                                        \
//            /* 64-Bit Host-State Fields     (values of --> 3 <-- in bits 11:10 indicates a host-state area) */                                                                          \
//        case VMCS_HOST_IA32_PAT_FULL_ENCODING:                                  VMX_SHORT_LOG("VMCS_HOST_IA32_PAT_FULL_ENCODING");                                                      \
//        case VMCS_HOST_IA32_EFER_FULL_ENCODING:                                 VMX_SHORT_LOG("VMCS_HOST_IA32_EFER_FULL_ENCODING");                                                     \
//        case VMCS_HOST_IA32_BNDCFGS_FULL_ENCODING:                              VMX_SHORT_LOG("VMCS_HOST_IA32_BNDCFGS_FULL_ENCODING");                                                  \
//                                                                                                                                                                                        \
//            /* 32-Bit Fields                (value of --> 2 <-- in bits 14:13 indicates a 32-bit field encoding) */                                                                     \
//                                                                                                                                                                                        \
//            /* 32-Bit Control Field         (value of --> 0 <-- in bits 11:10 indicates a 32-bit control field) */                                                                      \
//        case VMCS_PIN_BASED_VM_EXECUTION_CONTROL_ENCODING:              VMX_SHORT_LOG("VMCS_PIN_BASED_VM_EXECUTION_CONTROL_ENCODING");                                                  \
//        case VMCS_PRIMARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING:      VMX_SHORT_LOG("VMCS_PRIMARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING");                                          \
//        case VMCS_EXCEPTION_BITMAP_ENCODING:                            VMX_SHORT_LOG("VMCS_EXCEPTION_BITMAP_ENCODING");                                                                \
//        case VMCS_PAGE_FAULT_ERROR_CODE_MASK_ENCODING:                  VMX_SHORT_LOG("VMCS_PAGE_FAULT_ERROR_CODE_MASK_ENCODING");                                                      \
//        case VMCS_PAGE_FAULT_ERROR_CODE_MATCH_ENCODING:                 VMX_SHORT_LOG("VMCS_PAGE_FAULT_ERROR_CODE_MATCH_ENCODING");                                                     \
//        case VMCS_CR3_TARGET_COUNT_ENCODING:                            VMX_SHORT_LOG("VMCS_CR3_TARGET_COUNT_ENCODING");                                                                \
//        case VMCS_VM_EXIT_CONTROLS_ENCODING:                            VMX_SHORT_LOG("VMCS_VM_EXIT_CONTROLS_ENCODING");                                                                \
//        case VMCS_VM_EXIT_MSR_STORE_COUNT_ENCODING:                     VMX_SHORT_LOG("VMCS_VM_EXIT_MSR_STORE_COUNT_ENCODING");                                                         \
//        case VMCS_VM_EXIT_MSR_LOAD_COUNT_ENCODING:                      VMX_SHORT_LOG("VMCS_VM_EXIT_MSR_LOAD_COUNT_ENCODING");                                                          \
//        case VMCS_VM_ENTRY_CONTROLS_ENCODING:                           VMX_SHORT_LOG("VMCS_VM_ENTRY_CONTROLS_ENCODING");                                                               \
//        case VMCS_VM_ENTRY_MSR_LOAD_COUNT_ENCODING:                     VMX_SHORT_LOG("VMCS_VM_ENTRY_MSR_LOAD_COUNT_ENCODING");                                                         \
//        case VMCS_VM_ENTRY_INTERRUPTION_INFORMATION_FIELD_ENCODING:     VMX_SHORT_LOG("VMCS_VM_ENTRY_INTERRUPTION_INFORMATION_FIELD_ENCODING");                                         \
//        case VMCS_VM_ENTRY_EXCEPTION_ERROR_CODE_ENCODING:               VMX_SHORT_LOG("VMCS_VM_ENTRY_EXCEPTION_ERROR_CODE_ENCODING");                                                   \
//        case VMCS_VM_ENTRY_INSTRUCTION_LENGTH_ENCODING:                 VMX_SHORT_LOG("VMCS_VM_ENTRY_INSTRUCTION_LENGTH_ENCODING");                                                     \
//        case VMCS_SECONDARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING:    VMX_SHORT_LOG("VMCS_SECONDARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING");                                        \
//        case VMCS_PLE_GAP_ENCODING:                                     VMX_SHORT_LOG("VMCS_PLE_GAP_ENCODING");                                                                         \
//        case VMCS_PLE_WINDOW_ENCODING:                                  VMX_SHORT_LOG("VMCS_PLE_WINDOW_ENCODING");                                                                      \
//                                                                                                                                                                                        \
//            /* 32-Bit Read-Only Data Fields (value of --> 1 <-- in bits 11:10 indicates a read-only data field) */                                                                      \
//        case VMCS_VM_INSTRUCTION_ERROR_ENCODING:                        VMX_SHORT_LOG("VMCS_VM_INSTRUCTION_ERROR_ENCODING");                                                            \
//        case VMCS_EXIT_REASON_ENCODING:                                 VMX_SHORT_LOG("VMCS_EXIT_REASON_ENCODING");                                                                     \
//        case VMCS_VM_EXIT_INTERRUPTION_INFORMATION_ENCODING:            VMX_SHORT_LOG("VMCS_VM_EXIT_INTERRUPTION_INFORMATION_ENCODING");                                                \
//        case VMCS_VM_EXIT_INTERRUPTION_ERROR_CODE_ENCODING:             VMX_SHORT_LOG("VMCS_VM_EXIT_INTERRUPTION_ERROR_CODE_ENCODING");                                                 \
//        case VMCS_IDT_VECTORING_INFORMATION_FIELD_ENCODING:             VMX_SHORT_LOG("VMCS_IDT_VECTORING_INFORMATION_FIELD_ENCODING");                                                 \
//        case VMCS_IDT_VECTORING_ERROR_CODE_ENCODING:                    VMX_SHORT_LOG("VMCS_IDT_VECTORING_ERROR_CODE_ENCODING");                                                        \
//        case VMCS_VM_EXIT_INSTRUCTION_LENGTH_ENCODING:                  VMX_SHORT_LOG("VMCS_VM_EXIT_INSTRUCTION_LENGTH_ENCODING");                                                      \
//        case VMCS_VM_EXIT_INSTRUCTION_INFORMATION_ENCODING:             VMX_SHORT_LOG("VMCS_VM_EXIT_INSTRUCTION_INFORMATION_ENCODING");                                                 \
//                                                                                                                                                                                        \
//            /* 32-Bit Guest-Sate Fields    (value of --> 2 <-- in bits 11:10 indicates a field in the guest-state area) */                                                              \
//        case VMCS_GUEST_ES_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_ES_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_CS_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_CS_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_SS_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_SS_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_DS_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_DS_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_FS_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_FS_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_GS_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_GS_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_LDTR_LIMIT_ENCODING:                    VMX_SHORT_LOG("VMCS_GUEST_LDTR_LIMIT_ENCODING");                                                                        \
//        case VMCS_GUEST_TR_LIMIT_ENCODING:                      VMX_SHORT_LOG("VMCS_GUEST_TR_LIMIT_ENCODING");                                                                          \
//        case VMCS_GUEST_GDTR_LIMIT_ENCODING:                    VMX_SHORT_LOG("VMCS_GUEST_GDTR_LIMIT_ENCODING");                                                                        \
//        case VMCS_GUEST_IDTR_LIMIT_ENCODING:                    VMX_SHORT_LOG("VMCS_GUEST_IDTR_LIMIT_ENCODING");                                                                        \
//        case VMCS_GUEST_ES_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_ES_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_CS_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_CS_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_SS_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_SS_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_DS_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_DS_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_FS_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_FS_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_GS_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_GS_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_LDTR_ACCESS_RIGHTS_ENCODING:            VMX_SHORT_LOG("VMCS_GUEST_LDTR_ACCESS_RIGHTS_ENCODING");                                                                \
//        case VMCS_GUEST_TR_ACCESS_RIGHTS_ENCODING:              VMX_SHORT_LOG("VMCS_GUEST_TR_ACCESS_RIGHTS_ENCODING");                                                                  \
//        case VMCS_GUEST_INTERRUPTIBILITY_STATE_ENCODING:        VMX_SHORT_LOG("VMCS_GUEST_INTERRUPTIBILITY_STATE_ENCODING");                                                            \
//        case VMCS_GUEST_ACTIVITY_STATE_ENCODING:                VMX_SHORT_LOG("VMCS_GUEST_ACTIVITY_STATE_ENCODING");                                                                    \
//        case VMCS_GUEST_SMBASE_ENCODING:                        VMX_SHORT_LOG("VMCS_GUEST_SMBASE_ENCODING");                                                                            \
//        case VMCS_GUEST_IA32_SYSENTER_CS_ENCODING:              VMX_SHORT_LOG("VMCS_VM_EXIT_INSTRUCTION_INFORMATION_ENCODING");                                                         \
//        case VMCS_GUEST_VMX_PREEMPTION_TIMER_ENCODING:          VMX_SHORT_LOG("VMCS_GUEST_IA32_SYSENTER_CS_ENCODING");                                                                  \
//                                                                                                                                                                                        \
//              /* 32-Bit Host-State Fields  (value of --> 3 <-- in bits 11:10 indicates a field in the host-state area) */                                                               \
//                                                                                                                                                                                        \
//        case VMCS_HOST_IA32_SYSENTER_CS_ENCODING:                                                                                                                                       \
//                                                                                                                                                                                        \
//            /* Natural-Width Fields         (value of --> 3 <-- in bits 14:13 indicates a natural-width field) */                                                                       \
//                                                                                                                                                                                        \
//            /* Natural-Width Control Feilds (value of --> 0 <-- in bits 11:10 indicates a control field)*/                                                                              \
//        case VMCS_CR0_GUEST_HOST_MASK_ENCODING:                 VMX_SHORT_LOG("VMCS_CR0_GUEST_HOST_MASK_ENCODING");                                                                     \
//        case VMCS_CR4_GUEST_HOST_MASK_ENCODING:                 VMX_SHORT_LOG("VMCS_CR4_GUEST_HOST_MASK_ENCODING");                                                                     \
//        case VMCS_CR0_READ_SHADOW_ENCODING:                     VMX_SHORT_LOG("VMCS_CR0_READ_SHADOW_ENCODING");                                                                         \
//        case VMCS_CR4_READ_SHADOW_ENCODING:                     VMX_SHORT_LOG("VMCS_CR4_READ_SHADOW_ENCODING");                                                                         \
//        case VMCS_CR3_TARGET_VALUE_0_ENCODING:                  VMX_SHORT_LOG("VMCS_CR3_TARGET_VALUE_0_ENCODING");                                                                      \
//        case VMCS_CR3_TARGET_VALUE_1_ENCODING:                  VMX_SHORT_LOG("VMCS_CR3_TARGET_VALUE_1_ENCODING");                                                                      \
//        case VMCS_CR3_TARGET_VALUE_2_ENCODING:                  VMX_SHORT_LOG("VMCS_CR3_TARGET_VALUE_2_ENCODING");                                                                      \
//        case VMCS_CR3_TARGET_VALUE_3_ENCODING:                  VMX_SHORT_LOG("VMCS_CR3_TARGET_VALUE_3_ENCODING");                                                                      \
//                                                                                                                                                                                        \
//            /* Natural-Width Read-Only Data Fields (value of --> 1 <-- in bits 11:10 indicates a read-only data field) */                                                               \
//        case VMCS_EXIT_QUALIFICATION_ENCODING:                  VMX_SHORT_LOG("VMCS_EXIT_QUALIFICATION_ENCODING");                                                                      \
//        case VMCS_IO_RCX_ENCODING:                              VMX_SHORT_LOG("VMCS_IO_RCX_ENCODING");                                                                                  \
//        case VMCS_IO_RSI_ENCODING:                              VMX_SHORT_LOG("VMCS_IO_RSI_ENCODING");                                                                                  \
//        case VMCS_IO_RDI_ENCODING:                              VMX_SHORT_LOG("VMCS_IO_RDI_ENCODING");                                                                                  \
//        case VMCS_IO_RIP_ENCODING:                              VMX_SHORT_LOG("VMCS_IO_RIP_ENCODING");                                                                                  \
//        case VMCS_GUEST_LINIAR_ADDRESS_ENCODING:                VMX_SHORT_LOG("VMCS_GUEST_LINIAR_ADDRESS_ENCODING");                                                                    \
//                                                                                                                                                                                        \
//            /* Natural-Width Guest-State Fields (value of --> 2 <-- in bits 11:10 indicates a guest-state area) */                                                                      \
//        case VMCS_GUEST_CR0_ENCODING:                           VMX_SHORT_LOG("VMCS_GUEST_CR0_ENCODING");                                                                               \
//        case VMCS_GUEST_CR3_ENCODING:                           VMX_SHORT_LOG("VMCS_GUEST_CR3_ENCODING");                                                                               \
//        case VMCS_GUEST_CR4_ENCODING:                           VMX_SHORT_LOG("VMCS_GUEST_CR4_ENCODING");                                                                               \
//        case VMCS_GUEST_ES_BASE_ENCODING:                       VMX_SHORT_LOG("VMCS_GUEST_ES_BASE_ENCODING");                                                                           \
//        case VMCS_GUEST_CS_BASE_ENCODING:                       VMX_SHORT_LOG("VMCS_GUEST_CS_BASE_ENCODING");                                                                           \


//
// In case of an UNSUCCESSFULL writing of a MSR value, WARN it
//
#define WARN_IF_FAILED(ENCODING, STATUS, OP)                                        \
{                                                                                   \
    do                                                                              \
    {                                                                               \
        if (!NT_SUCCESS(STATUS))                                                    \
        {                                                                           \
            if (OP == VMX_WRITE)                                                    \
            {                                                                       \
                LOG_WARNING(ENCODING " unsuccessfull");                             \
            }                                                                       \
            else if (OP == VMX_READ)                                                \
            {                                                                       \
                LOG("Reading from " ENCODING " failed");                            \
            }                                                                       \
        }                                                                           \
    } while (0);                                                                    \
}

// 
// Perfroms VmxWrite with all checks (OP - operation to perform: vmx_read, vmx_write)
// It should support all (read, write, clear .. )
// 

#pragma warning( disable : 4022 )
#pragma warning( disable : 4047 ) /// 
#pragma warning( disable : 4312 ) /// Expander macro, not all variables visibile yet (Conversion to greader size..)
#pragma warning( disable : 4306 ) /// Conversion from enum..
#pragma warning( disable : 4133)

#define VMX_OPERATION(ENCODING, VALUE, OP)                                          \
{                                                                                   \
    do                                                                              \
    {                                                                               \
        if (VMCS_LOGS)                                                              \
        {                                                                           \
            VMX_SHORT_LOG(#ENCODING, ((OP == VMX_WRITE) ? (VALUE) : (#VALUE)), OP); \
        }                                                                           \
        WARN_IF_FAILED(#ENCODING,                                                   \
                       (OP == VMX_WRITE) ?                                          \
                              (VmxWrite(ENCODING, VALUE)) :                         \
                              (VmxRead(NULL, ENCODING, (uintptr_t*)VALUE)),         \
                       OP);                                                         \
    } while (0);                                                                    \
}


//
// Exit qualification for Control Registers (R8 through R15 used only if the processor supports Intel 64 architecture)
//
typedef enum _HV_CONTROL_REGISTER
{
    CR_RAX,     // = 0
    CR_RCX,     // = 1
    CR_RDX,     // = 2
    CR_RBX,     // = 3
    CR_RSP,     // = 4
    CR_RBP,     // = 5
    CR_RSI,     // = 6
    CR_RDI,     // = 7
    CR_R8,      // = 8
    CR_R9,      // = 9
    CR_R10,     // = 10
    CR_R11,     // = 11
    CR_R12,     // = 12
    CR_R13,     // = 13
    CR_R14,     // = 14
    CR_R15      // = 15
}HV_CONTROL_REGISTER;

extern void _start_guest_16();
extern void _end_guest_16();
extern void SaveGuestRegisters();
extern void RestoreGuestRegistersAndResume(PPROCESSOR_STATE);
extern void _start_hooked_int15h();
extern void _end_hooked_int15h();

static EPTP m_eptp;

static IDT_ENTRY_REAL_MODE m_originalInt15hIdtEntry;

#define VMX_VXMON_REGION_SIZE       0x1000 // 4Kilo

__forceinline
VOID
Cr4SetBit(
    _In_ BYTE bit
    )
{
    QWORD cr4;
    cr4 = __readcr4();


    cr4 = cr4 | (GET_MASK_OF_1_BIT_SET(bit));
    LOG_INFO("CR4: 0x%08X", cr4);
    __writecr4(cr4);
}

__forceinline
VOID
MsrSetBit(
    _In_ DWORD MsrAdress,
    _In_ QWORD BitIndex
)
{
    QWORD msr = __readmsr(MsrAdress);

    __writemsr(MsrAdress, msr | (1ULL < BitIndex));
}

NTSTATUS
CheckVmxSupported(
    VOID
    )
{
    BOOLEAN vmxSupported;
    BOOLEAN isSupported;
    BYTE bit;

    vmxSupported = FALSE;
    INT cpuInfo[DEF_CPU_INFO];
    __cpuid(cpuInfo, 0x01);

    for (bit = 0; bit < SIZE_DWORD; bit++)
    {
        isSupported = Check_ECX(cpuInfo[ECX], bit);
        if (bit == VMX)
        {
            vmxSupported = TRUE;
        }
    }

    return (vmxSupported) ? (STATUS_SUCCESS) : (STATUS_VMX_NOT_SUPPORTED);
}

VOID
DetermineN(
    VOID
)
{
    INT cpuInfo[DEF_CPU_INFO];
    __cpuid(cpuInfo, 0x80000008);
    LOG_INFO("MAX N: 0x%X", (cpuInfo[EAX] >> 8) & 0xFFULL);
}


QWORD
ReadMsr(
    _In_ DWORD Address,
    _In_ QWORD Bit,
    _In_ QWORD Mask

)
{
    return ((__readmsr(Address) >> Bit) & Mask);
}

QWORD
ReadFullMsr(
    _In_ DWORD Address
)
{
    return (__readmsr(Address) >> 32);
}

static
VOID
VmxSetRestrictedValues(
    VOID
    )
{
    LOG_INFO("Setting restricted values");

    QWORD msrIa32VmxCr0Fixed0 = __readmsr(MSR_IA32_VMX_CR0_FIXED0);
    QWORD msrIa32VmxCr0Fixed1 = __readmsr(MSR_IA32_VMX_CR0_FIXED1);

    LOG_MSR("Cr0Fixed0: 0%X", msrIa32VmxCr0Fixed0);
    LOG_MSR("Cr0Fixed1: 0%X", msrIa32VmxCr0Fixed1);

    QWORD msrIa32VmxCr4Fixed0 = __readmsr(MSR_IA32_VMX_CR4_FIXED0);
    QWORD msrIa32VmxCr4Fixed1 = __readmsr(MSR_IA32_VMX_CR4_FIXED1);

    __writecr0((__readcr0() & msrIa32VmxCr0Fixed1) | (msrIa32VmxCr0Fixed0));
    __writecr4((__readcr4() & msrIa32VmxCr4Fixed1) | (msrIa32VmxCr4Fixed0));

    LOG_INFO("Successfully set restricted values");
}

NTSTATUS
VmxStatusToNtStatus(
    VMX_STATUS VmxStatus
)
{
    switch (VmxStatus)
    {
    case VMX_Success:                   return STATUS_SUCCESS;
    case VMX_FailureWithErrorCode:      return STATUS_VMX_FAILED_WITH_ERR_CODE;
    case VMX_FailureWithoutErrorCode:   return STATUS_VMX_FAILED_WITHOUT_ERR_CODE;
    default:                            return STATUS_VMX_NOT_SUPPORTED;
    }
}


VOID
VmxSetRevisionIndentifier(
    _In_ PVMCS VmxRegion
)
{
    QWORD basicVmxInfo = __readmsr(MSR_IA32_VMX_BASIC);

    LOG_MSR("BasicVmxInfo: 0x%08X", basicVmxInfo);
    LOG_INFO("VMCS_REVISION_IDENT 0x%08X", VmxRegion->RevIdent.VMCS_REVISION_IDENT);

    VmxRegion->RevIdent.VMCS_REVISION_IDENT = (DWORD)basicVmxInfo;
    VmxRegion->RevIdent.VMCS_SHADOW_INDICATOR = 0;
}


NTSTATUS
VmxOn(
    PVOID VmcsPhysicalAddress
)
{
    VMX_STATUS vmxStatus;

    vmxStatus = __vmx_on(VmcsPhysicalAddress);
    LOG_MSR("VMX STATUS VALUE: %d", vmxStatus);

    return VmxStatusToNtStatus(vmxStatus);

}

NTSTATUS
VmxClear(
    _In_ PVOID VmcsPhysicalAddress
)
{
    VMX_STATUS vmxStatus;

    vmxStatus = __vmx_vmclear(VmcsPhysicalAddress);
    LOG_MSR("VMX STATUS VALUE: %d", vmxStatus);

    return VmxStatusToNtStatus(vmxStatus);
}

NTSTATUS
VmxPtrld(
    _In_ PVOID VmcsPhysicalAddress
)
{
    VMX_STATUS vmxStatus;

    vmxStatus = __vmx_vmptrld(VmcsPhysicalAddress);
    LOG_MSR("VMX STATUS VALUE: %d", vmxStatus);

    return VmxStatusToNtStatus(vmxStatus);
}


DWORD
CheckExecutionControls(
    DWORD MsrToCheck
)
{
    // BITS 31:0  -> 0-settings
    // BITS 63:32 -> 1-settings


    QWORD nonTrueMsrValue;
    DWORD vmExecutionControl;

    DWORD allowedZeroSettings = 0;
    DWORD allowedOneSettings = 0;

    nonTrueMsrValue = __readmsr(MsrToCheck);

    allowedZeroSettings = (DWORD)(nonTrueMsrValue & MAX_DWORD);
    allowedOneSettings = (DWORD)(nonTrueMsrValue >> MSR_ALLOWED1_SETTINGS_SHIFT);

    LOG_INFO("MSR Val: 0x%08x", MsrToCheck);
    LOG_INFO("Allowed zero settings: 0x%08x", allowedZeroSettings);
    LOG_INFO("Allowed one settings: 0x%08x", allowedOneSettings);

    vmExecutionControl = allowedOneSettings & allowedZeroSettings;

    return vmExecutionControl;
}

VOID
ApplySettings(
    BYTE msrIa32VmxBasicBit55
)
{
    QWORD msrIa32VmxPinbasedCtls;
    QWORD msrIa32VmxProcbasedCtls;
    QWORD msrIa32VmxProcbasedCtls2;

    QWORD msrIa32VmxTruePinbasedCtls;

    switch (msrIa32VmxBasicBit55)
    {
    case 0:
        break;
    case 1:
    {
        msrIa32VmxPinbasedCtls = ReadMsr(MSR_IA32_VMX_PINBASED_CTLS, 0, MAX_QWORD);
        LOG_INFO("MSR_IA32_VMX_PINBASED_CTLS: 0x%08X", msrIa32VmxPinbasedCtls);

        msrIa32VmxTruePinbasedCtls = ReadMsr(MSR_IA32_VMX_TRUE_PINBASED_CTLS, 0, MAX_QWORD);
        LOG_INFO("MSR_IA32_VMX_TRUE_PINBASED_CTLS: 0x%08X", msrIa32VmxTruePinbasedCtls);

        msrIa32VmxProcbasedCtls = ReadMsr(MSR_IA32_VMX_PROCBASED_CTLS, 0, MAX_QWORD);
        LOG_INFO("MSR_IA32_VMX_PROCBASED_CTLS: 0x%08X", msrIa32VmxProcbasedCtls);

        msrIa32VmxProcbasedCtls2 = ReadMsr(MSR_IA32_VMX_PROCBASED_CTLS2, 0, MAX_QWORD);
        LOG_INFO("MSR_IA32_VMX_PROCBASED_CTLS2: 0x%08X", msrIa32VmxProcbasedCtls2);

        break;
    }
    }
}

//VOID
//SetBreak(
//    BOOLEAN Opt
//)
//{
//    (Opt == TRUE) ? (gPutBreak = TRUE) : (gPutBreak = FALSE);
//}

NTSTATUS
VmxWrite(
    QWORD Enc,
    QWORD Val
)
{
    VMX_STATUS vmxStatus;
    vmxStatus = __vmx_vmwrite(Enc, Val);

    IncrementLoaded(1, SET_LOAD_BAR);
    IncrementLoaded(1, SET_MINIHV_BAR);
    return VmxStatusToNtStatus(vmxStatus);
}

#define CR0_PG_BIT_INDEX 31
#define CR0_PE_BIT_INDEX 0

VOID
VmxSetVmcsGuestHostMasksAndReadShadowsForCr0AndCr4(
    VOID
)
{
    QWORD msrIa32VmxCr0Fixed0 = __readmsr(MSR_IA32_VMX_CR0_FIXED0);
    QWORD msrIa32VmxCr0Fixed1 = __readmsr(MSR_IA32_VMX_CR0_FIXED1);
    QWORD msrIa32VmxCr4Fixed0 = __readmsr(MSR_IA32_VMX_CR4_FIXED0);
    QWORD msrIa32VmxCr4Fixed1 = __readmsr(MSR_IA32_VMX_CR4_FIXED1);

    CLEAR_BITT(msrIa32VmxCr0Fixed0, CR0_PG_BIT_INDEX);
    CLEAR_BITT(msrIa32VmxCr0Fixed0, CR0_PE_BIT_INDEX);

    QWORD cr0ReadShadow = msrIa32VmxCr0Fixed0 & msrIa32VmxCr0Fixed1;
    QWORD cr4ReadShadow = msrIa32VmxCr4Fixed0 & msrIa32VmxCr4Fixed1;
    QWORD cr0Mask = (msrIa32VmxCr0Fixed0 | ~msrIa32VmxCr0Fixed1) & MAX_DWORD;
    QWORD cr4Mask = (msrIa32VmxCr4Fixed0 | ~msrIa32VmxCr4Fixed1) & MAX_DWORD;

    VMX_OPERATION(VMCS_CR0_READ_SHADOW_ENCODING,     cr0ReadShadow, VMX_WRITE);
    VMX_OPERATION(VMCS_CR4_READ_SHADOW_ENCODING,     cr4ReadShadow, VMX_WRITE);
    VMX_OPERATION(VMCS_CR0_GUEST_HOST_MASK_ENCODING, cr0Mask,       VMX_WRITE);
    VMX_OPERATION(VMCS_CR4_GUEST_HOST_MASK_ENCODING, cr4Mask,       VMX_WRITE);
}

VOID
VmxSetVmcsMsrBitmaps(
    VOID
)
{
    PVOID msrBitmap;

    msrBitmap = MmuAllocVa(VMX_MSR_BITMAPS_SIZE);
    memzero(msrBitmap, VMX_MSR_BITMAPS_SIZE);

    VMX_OPERATION(VMCS_ADDRESS_OF_MSR_BITMAPS_FULL_ENCODING, (QWORD)VA2PA(msrBitmap), VMX_WRITE);
}

VOID
VmxSetVmcsCr3TargetControls(
    VOID
)
{
    QWORD cr3TargetValues[4] = { 0 };
    DWORD cr3TargetCount = 0;

    VMX_OPERATION(VMCS_CR3_TARGET_COUNT_ENCODING,   cr3TargetCount,     VMX_WRITE);
    VMX_OPERATION(VMCS_CR3_TARGET_VALUE_0_ENCODING, cr3TargetValues[0], VMX_WRITE);
    VMX_OPERATION(VMCS_CR3_TARGET_VALUE_1_ENCODING, cr3TargetValues[1], VMX_WRITE);
    VMX_OPERATION(VMCS_CR3_TARGET_VALUE_3_ENCODING, cr3TargetValues[3], VMX_WRITE);
}

#define EPT_PAGE_WALK 4
#define EPT_PML4_PA (PML4_ADDRESS + 10 * MB)

#define EPT_PHYSICAL_ADDRESS_SHIFT 12



VOID
VmxSetVmcsEptp(
    VOID
)
{
    m_eptp.Raw = 0;

    if (ReadMsr(MSR_IA32_VMX_EPT_VPID_CAP, MSR_IA32_VMX_EPT_VPID_CAP_WB_BIT_INDEX, BIT_MASK))
    {
        m_eptp.MemoryType = WRITE_BACK_VALUE;
    }

    m_eptp.PgWalkLenMinusOne = EPT_PAGE_WALK - 1;

    m_eptp.PhysicalAddress = EPT_PML4_PA >> EPT_PHYSICAL_ADDRESS_SHIFT;

    VMX_OPERATION(VMCS_EPT_POINTER_FULL_ENCODING, m_eptp.Raw, VMX_WRITE);
}

VOID
VmxSetVmcsVmExecutionControls(
    VOID
)
{
    DWORD pinBasedVmExecutionControls;
    DWORD primaryProcessorBasedVmExecutionControls;
    DWORD secondaryProcessorBasedVmExecutionControls = 0;

    pinBasedVmExecutionControls = CheckExecutionControls(MSR_IA32_VMX_PINBASED_CTLS);

    VMX_OPERATION(VMCS_PIN_BASED_VM_EXECUTION_CONTROL_ENCODING, pinBasedVmExecutionControls, VMX_WRITE);

    primaryProcessorBasedVmExecutionControls = CheckExecutionControls(MSR_IA32_VMX_PROCBASED_CTLS);
    SET_BITT(primaryProcessorBasedVmExecutionControls, VMCS_ACTIVATE_SECONDARY_CONTROLS_BIT_INDEX);
    CLEAR_BITT(primaryProcessorBasedVmExecutionControls, VMCS_CR3_LOAD_EXITING_BIT_INDEX);
    CLEAR_BITT(primaryProcessorBasedVmExecutionControls, VMCS_CR3_STORE_EXITING_BIT_INDEX);
    CLEAR_BITT(primaryProcessorBasedVmExecutionControls, VMCS_CR8_LOAD_EXITING_BIT_INDEX);
    CLEAR_BITT(primaryProcessorBasedVmExecutionControls, VMCS_CR8_STORE_EXITING_BIT_INDEX);
    SET_BITT(primaryProcessorBasedVmExecutionControls, VMCS_USE_MSR_BITMAPS_BIT_INDEX);
    //SET_BITT(primaryProcessorBasedVmExecutionControls, VMCS_USE_IO_BITMAPS_BIT_INDEX);
    if (ACTIVATE_HLT_EXITING == 1)
    {
        SET_BITT(primaryProcessorBasedVmExecutionControls, VMX_PRIMARY_PROCESSOR_BASED_CONTROLS_HLT_EXITING_BIT_INDEX);
    }
    VMX_OPERATION(VMCS_PRIMARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING, primaryProcessorBasedVmExecutionControls, VMX_WRITE);
  
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_ENABLE_EPT_BIT_INDEX);
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_ENABLE_RDTSCP_BIT_INDEX);
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_ENABLE_INVPCID_BIT_INDEX);
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_ENABLE_XSAVES_XRSTORS_BIT_INDEX);
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_ENABLE_EPT_BIT_INDEX);
    SET_BITT(secondaryProcessorBasedVmExecutionControls, VMCS_UNRESTRICTED_GUEST_BIT_INDEX);
    VMX_OPERATION(VMCS_SECONDARY_PROCESSOR_VM_EXECUTION_CONTROL_ENCODING, secondaryProcessorBasedVmExecutionControls, VMX_WRITE);


    VmxSetVmcsGuestHostMasksAndReadShadowsForCr0AndCr4();

    VmxSetVmcsMsrBitmaps();

    VmxSetVmcsCr3TargetControls();

    VmxSetVmcsEptp();
}



VOID
VmxSetVmcsVmEntryControlsForMsrs(
    VOID
)
{
    DWORD vmEntryMsrLoadCount = 0;
    QWORD vmEntryMsrLoadAddress = 0;

    VMX_OPERATION(VMCS_VM_ENTRY_MSR_LOAD_COUNT_ENCODING, vmEntryMsrLoadCount, VMX_WRITE);
    
    UNREFERENCED_PARAMETER(vmEntryMsrLoadAddress);
}

VOID
VmxSetVmcsVmEntryControlsForEventInjection(
    VOID
)
{
    VM_ENTRY_INTERRUPTION_INFORMATION_FIELD vmEntryInterruptionInformationField = { 0 };

    VMX_OPERATION(VMCS_VM_ENTRY_INTERRUPTION_INFORMATION_FIELD_ENCODING, vmEntryInterruptionInformationField.Raw, VMX_WRITE);
}


VOID
VmxSetVmcsVmEntryControls(
    VOID
)
{
    DWORD vmEntryControls;

    vmEntryControls = CheckExecutionControls(MSR_IA32_VMX_ENTRY_CTLS);
    CLEAR_BITT(vmEntryControls, VMX_LOAD_DEBUG_CONTROLS_BIT_INDEX);
    SET_BITT(vmEntryControls, VMX_ENTRY_LOAD_IA32_EFER_BIT_INDEX);
    VMX_OPERATION(VMCS_VM_ENTRY_CONTROLS_ENCODING, vmEntryControls, VMX_WRITE);
    
    VmxSetVmcsVmEntryControlsForMsrs();
    VmxSetVmcsVmEntryControlsForEventInjection();
}


VOID
VmxSetVmcsVmExitControlsForMsrs(
    VOID
)
{
    DWORD vmExitMsrStoreCount = 0;
    DWORD vmExitMsrLoadCount = 0;

    VMX_OPERATION(VMCS_VM_EXIT_MSR_STORE_COUNT_ENCODING, vmExitMsrStoreCount, VMX_WRITE);
    VMX_OPERATION(VMCS_VM_EXIT_MSR_LOAD_COUNT_ENCODING, vmExitMsrLoadCount, VMX_WRITE);
}

VOID
VmxSetVmcsVmExitControls(
    VOID
)
{
    DWORD vmExitControls;

    vmExitControls = CheckExecutionControls(MSR_IA32_VMX_EXIT_CTLS);
    SET_BITT(vmExitControls, VMCS_SAVE_IA32_EFER_BIT_INDEX);
    SET_BITT(vmExitControls, VMCS_LOAD_IA32_EFER_BIT_INDEX);
    SET_BITT(vmExitControls, VMCS_HOST_ADDRESS_SPACE_SIZE_BIT_INDEX);
    VMX_OPERATION(VMCS_VM_EXIT_CONTROLS_ENCODING, vmExitControls, VMX_WRITE);

    VmxSetVmcsVmExitControlsForMsrs();
}

VOID
VmxSetVmcsVmxControls(
    VOID
)
{
    BYTE msrIa32VmxBit55;

    msrIa32VmxBit55 = (BYTE)ReadMsr(MSR_IA32_VMX_BASIC, 55, 0x1);

    LOG_INFO("Bit 55 value: %d", msrIa32VmxBit55);


    LOG_INFO("Bit 55 of MSR IA32_VMX_BASIC found 1");
    LOG_INFO("True msr");
    VmxSetVmcsVmExecutionControls();
    VmxSetVmcsVmEntryControls();
    VmxSetVmcsVmExitControls();

}

VOID
VmxSetVmcsHostControlRegistersAndMsrs(
    VOID
)
{
    VMX_OPERATION(VMCS_HOST_CR0_ENCODING, __readcr0(),     VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_CR3_ENCODING, __readcr3(),     VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_CR4_ENCODING, __readcr4(),     VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_IA32_SYSENTER_ESP_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_IA32_SYSENTER_EIP_ENCODING, 0, VMX_WRITE);

}


VOID
VmxSetVmcsHostRipAndRsp(
    PCPU Cpu
)
{
    Cpu->StackTop = MmuAllocVa(PAGE_SIZE);
    VMX_OPERATION(VMCS_HOST_RSP_ENCODING, (QWORD)Cpu->StackTop,      VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_RIP_ENCODING, (QWORD)SaveGuestRegisters, VMX_WRITE);
}

VOID
VmxSetVmcsHostSegmentAndDescriptorTableRegisters(
    PCPU Cpu
)
{
    VMX_OPERATION(VMCS_HOST_ES_ENCODING,        0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_CS_ENCODING,        (QWORD)GDT_CODE64_SEL,          VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_SS_ENCODING,        0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_DS_ENCODING,        0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_FS_ENCODING,        0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_GS_ENCODING,        0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_TR_ENCODING,        (QWORD)Cpu->TrSelector,         VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_FS_BASE_ENCODING,   0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_GS_BASE_ENCODING,   0,                              VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_TR_BASE_ENCODING,   (QWORD)&Cpu->Tss,               VMX_WRITE);
    VMX_OPERATION(VMCS_HOST_GDTR_BASE_ENCODING, (QWORD)GdtGetGdtrBaseAddress(), VMX_WRITE);

    //VMX_OPERATION(VMCS_HOST_IDTR_BASE_ENCODING, (QWORD)IdtGetIdtrBaseAddress(), VMX_WRITE);
}


VOID
VmxSetVmcsHostArea(
    PCPU Cpu
    )
{
    QWORD eferVal = ReadMsr(MSR_IA32_EFER, 0, MAX_QWORD);

    VmxSetVmcsHostControlRegistersAndMsrs();
    VmxSetVmcsHostRipAndRsp(Cpu);
    VmxSetVmcsHostSegmentAndDescriptorTableRegisters(Cpu);

    VMX_OPERATION(VMCS_HOST_IA32_EFER_FULL_ENCODING, eferVal, VMX_WRITE);
}


NTSTATUS
VmxRead(
    char *EncodingName,
    QWORD Encoding,
    PVOID Destination
)
{
    VMX_STATUS vmxStatus;
    NTSTATUS status;

    vmxStatus = __vmx_vmread(Encoding, Destination);
    status = VmxStatusToNtStatus(vmxStatus);

    if (EncodingName != NULL)
    {
        LOG("Value of %s (0x%X) is 0x%X", EncodingName, Encoding, *(PQWORD)Destination);
    }

    if (!NT_SUCCESS(status))
    {
        LOG("Reading from %s failed", EncodingName ? EncodingName : "Unknown");
    }

    return status;
}

NTSTATUS
VmxOff(
    VOID
)
{

    LOG("Exiting vmx");
    __vmx_off();


    return STATUS_SUCCESS;
}

//BOOLEAN
//PciBDFEqualsReservedSerialPort(
//    BYTE Bus,
//    BYTE Device,
//    BYTE Function
//)
//{
//    return (m_reservedSerialPortConfigAddresses.BusNumber == Bus) && (m_reservedSerialPortConfigAddresses.DeviceNumber == Device) && (m_reservedSerialPortConfigAddresses.FunctionNumber == Function);
//}
VOID
VmxSetBitInBitmap(
    PBYTE Bitmap,
    WORD BitIndex,
    BOOLEAN SetBit
)
{
    QWORD byteOffset = (BitIndex & (~0x7)) >> 3;
    BYTE bitOffset = BitIndex & 0x7;

    if (SetBit)
    {
        SET_BITT(Bitmap[byteOffset], bitOffset);
    }
    else
    {
        CLEAR_BITT(Bitmap[byteOffset], bitOffset);
    }
}


VOID
VmxJumpToNextInstr(
    VOID
)
{
    DWORD instructionLength;
    QWORD guestRip;

    VmxRead(NULL, VMCS_VM_EXIT_INSTRUCTION_LENGTH_ENCODING, &instructionLength);
    VmxRead(NULL, VMCS_GUEST_RIP_ENCODING, &guestRip);
    VMX_OPERATION(VMCS_GUEST_RIP_ENCODING, guestRip + instructionLength, VMX_WRITE);

    LOG_INFO("Succsefully jumped to next instr");
}


#define CPUID_01_RDX_SSE_SUPPORTED      0x19

#define CLEAR_BIT(x, b) (x &= (~(1 << b)))


NTSTATUS
VmxBasicExitReasonPreemptionTimerExpired(
    _In_ PPROCESSOR_STATE ProcessorState
    )
{

    UNREFERENCED_PARAMETER(ProcessorState);

    NTSTATUS ntStatus;
    ntStatus = STATUS_SUCCESS;
    LOG("Exit reason: VMX_BASIC_EXIT_REASON_PREEMPTION_TIMER_EXPIRED");


    return ntStatus;
}

NTSTATUS
VmxBasicExitReasonTaskSwitch(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);
    LOG("Exit reason: VMX_BASIC_EXIT_REASON_TASK_SWITCH");

    //
    // [TO DO]
    // -> Read exit qualification
    // -> (&)
    //

    QWORD exitQualificationTS;

    // Selector of task-state segment (TSS) to which guest attempted to switch
    WORD tss;

    // Source of task switch initiation (can be one of the following )
    //  -> 0: CALL instruction
    //  -> 1: IRET instruction
    //  -> 2: JMP instruction
    //  -> 3: Task gate in IDT
    BYTE sourceOfTaskSwitch;


    VmxRead(NULL, VMCS_EXIT_QUALIFICATION_ENCODING, &exitQualificationTS);

    // bits 15:09
    tss = exitQualificationTS & VMX_BASIC_EXIT_REASON_MASK;
    sourceOfTaskSwitch = GET_BITS(exitQualificationTS, 30, SOURCE_OF_TASK_INIT);
    LOG_INFO("VM EXIT TASK SWITCH: [%d]", sourceOfTaskSwitch);
    switch (sourceOfTaskSwitch)
    {
    case BIT_CALL_INSTR:    LOG("CALL");        break;
    case BIT_IRET_INSTR:    LOG("IRET");        break;
    case BIT_JMP_INSTR:     LOG("JMP");         break;
    case BIT_TASK_GATE_IDT: LOG("TASK GATE");   break;
    default:     break;
    }

    VmxOff();

    __writecr3(0);

    return STATUS_SUCCESS;
}

NTSTATUS
VmxBasicExitReasonIoInstruction(
    PPROCESSOR_STATE ProcessorState
    )
{
    UNREFERENCED_PARAMETER(ProcessorState);

    LOG("Exit reason: VMX_BASIC_EXIT_REASON_IO_INSTRUCTION");

    BYTE guestFunction = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_FUNCTION_SHIFT) & PCI_CONFIG_ADDRESS_FUNCTION_MASK;
    BYTE guestDevice = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_DEVICE_SHIFT) & PCI_CONFIG_ADDRESS_DEVICE_MASK;
    BYTE guestBus = (ProcessorState->RegisterValues[RegisterRax] >> PCI_CONFIG_ADDRESS_BUS_SHIFT) & PCI_CONFIG_ADDRESS_BUS_MASK;
    BYTE guestRegister = (ProcessorState->RegisterValues[RegisterRax] >> 2) & 0x3F;
    QWORD guestIoBitmapPa;
    PBYTE guestIoBitmapVa;

    if (ProcessorState->RegisterValues[RegisterRdx] == PCI_CONFIG_ADDRESS_PORT)
    {
        if (PciBDFEqualsReservedSerialPort(guestBus, guestDevice, guestFunction))
        {
            if (guestRegister == 0)
            {
                LOG("Guest tried to interogate PCI for our reserved serial port");
                VmxRead(NULL, VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING, &guestIoBitmapPa);
                guestIoBitmapVa = MmuMapToVa(guestIoBitmapPa, VMX_IO_BITMAP_SIZE, NULL, FALSE);
                VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_ADDRESS_PORT, FALSE); // disable exits on CONFIG_ADDRESS port accesses
                VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_DATA_PORT, TRUE); // enable exits on CONFIG_DATA port accesses
                MmuUnmapFromVa((QWORD)guestIoBitmapVa, VMX_IO_BITMAP_SIZE);
            }
        }
    }
    else if (ProcessorState->RegisterValues[RegisterRdx] == PCI_CONFIG_DATA_PORT)
    {
        LOG("Will send invalid vendor to guest");
        VmxRead(NULL, VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING, &guestIoBitmapPa);
        guestIoBitmapVa = MmuMapToVa(guestIoBitmapPa, VMX_IO_BITMAP_SIZE, NULL, FALSE);
        ProcessorState->RegisterValues[RegisterRax] = MAX_DWORD; // invalid vendor id
        VmxSetBitInBitmap(guestIoBitmapVa, PCI_CONFIG_DATA_PORT, FALSE); // disable exits on CONFIG_DATA port accesses
        MmuUnmapFromVa((QWORD)guestIoBitmapVa, VMX_IO_BITMAP_SIZE);
    }

    VmxJumpToNextInstr();

    return STATUS_SUCCESS;
}


NTSTATUS
VmxBasicExitReasonCpuid(
    PPROCESSOR_STATE ProcessorState
    )
{
    DWORD cpuInfo[4];
    QWORD guestCr4;

    __cpuidex(cpuInfo, ProcessorState->RegisterValues[RegisterRax], ProcessorState->RegisterValues[RegisterRcx]);

    if ((DWORD)ProcessorState->RegisterValues[RegisterRax] == 0x01)
    {
        //
        // check if guest set the OXSAVE bit, since CPUIDECX returns the OSXSAVE bit
        //

        VmxRead(NULL, VMCS_GUEST_CR4_ENCODING, &guestCr4);
        if (GET_BITS(guestCr4, CR4_OSXSAVE_BIT_INDEX, 1))
        {
            SET_BITT(cpuInfo[CPUID_RCX], CPUID_01H_RCX_OSXSAVE_BIT_INDEX);
        }
        else
        {
            CLEAR_BITT(cpuInfo[CPUID_RCX], CPUID_01H_RCX_OSXSAVE_BIT_INDEX);
        }
    }

    
    //
    // Cpuid executed with RAX leav 0x01
    // 
    
    if (ProcessorState->RegisterValues[RegisterRax] == 0x01)
    {
        //
        // Check for HYPER V PRESENT BIT
        //
        cpuInfo[2] |= HYPER_V_BIT;
    }
    else if (ProcessorState->RegisterValues[RegisterRax] == HYPER_V_INT)
    {
        //
        // Check for hyper-v interface
        //
        cpuInfo[0] = MHV_SIGN;
    }

    ProcessorState->RegisterValues[RegisterRax] = (QWORD)(cpuInfo[CPUID_RAX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRbx] = (QWORD)(cpuInfo[CPUID_RBX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRcx] = (QWORD)(cpuInfo[CPUID_RCX] & MAX_DWORD);
    ProcessorState->RegisterValues[RegisterRdx] = (QWORD)(cpuInfo[CPUID_RDX] & MAX_DWORD);

    VmxJumpToNextInstr();

    return STATUS_SUCCESS;
}

NTSTATUS
VmxBasicExitReasonXsetbv(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    QWORD edxEax;

    if (XcrSupportedValue(ProcessorState->RegisterValues[RegisterRcx]))
    {
        edxEax = (ProcessorState->RegisterValues[RegisterRdx] << 32) | (ProcessorState->RegisterValues[RegisterRax] & MAX_DWORD);
        _xsetbv(ProcessorState->RegisterValues[RegisterRcx], edxEax);
    }

    VmxJumpToNextInstr();

    return STATUS_SUCCESS;
}

NTSTATUS
VmxBasicExitReasonEptViolation(
    PPROCESSOR_STATE ProcessorState
    )
{
    /*****************************************************************************/
    /* EXIT Qualification for EPT Violations:  Chapter 27-2 VM-Exit Table (27-7) */
    /*****************************************************************************/

    // 0 - Set if the access causing the EPT violation was a data read.
    // 1 - Set if the access causing the EPT violation was a data write.
    // 2 - Set if the access causing the EPT violation was an instruction fetch.
    // 3 - The logical-AND of bit 0 in the EPT paging-structure entries used to translate the guest-physical address of the
    ///    access causing the EPT violation(indicates whether the guest - physical address was readable)
    // 4 - The logical-AND of bit 1 in the EPT paging-structure entries used to translate the guest-physical address of the
    ///    access causing the EPT violation (indicates whether the guest-physical address was writeable)
    // 5 - The logical-AND of bit 1 in the EPT paging-structure entries used to translate the guest-physical address of the
    ///    access causing the EPT violation(indicates whether the guest - physical address was writeable)
    // 6 - If the “mode-based execute control” VM-execution control is 0, the value of this bit is undefined. If that control is
    ///    1, this bit is the logical - AND of bit 10 in the EPT paging - structures entries used to translate the guest - physical
    ///    address of the access causing the EPT violation.In this case, it indicates whether the guest - physical address was
    ///    executable for user - mode linear addresses.
    // 7 - Set if the guest linear-address field is valid.
    ///    The guest linear - address field is valid for all EPT violations except those resulting from an attempt to load the
    ///    guest PDPTEs as part of the execution of the MOV CR instruction.
    // 8 - If bit 7 is 1:
    ///     • Set if the access causing the EPT violation is to a guest - physical address that is the translation of a linear
    ///       address.
    ///     • Clear if the access causing the EPT violation is to a paging - structure entry as part of a page walk or the
    ///       update of an accessed or dirty bit.
    ///       Reserved if bit 7 is 0 (cleared to 0)
    // 9 - If bit 7 is 1, bit 8 is 1, and the processor supports advanced VM-exit information for EPT violations,3 this bit is 0
    ///    if the linear address is a supervisor - mode linear address and 1 if it is a user - mode linear address. (If CR0.PG = 0,
    ///    the translation of every linear address is a user - mode linear address and thus this bit will be 1.) Otherwise, this
    ///    bit is undefined.
    // 10 - If bit 7 is 1, bit 8 is 1, and the processor supports advanced VM-exit information for EPT violations,3 this bit is 0
    ///     if paging translates the linear address to a read - only page and 1 if it translates to a read / write page. (If CR0.PG =
    ///     0, every linear address is read / write and thus this bit will be 1.) Otherwise, this bit is undefined.
    // 11 - If bit 7 is 1, bit 8 is 1, and the processor supports advanced VM-exit information for EPT violations,3 this bit is 0
    ///     if paging translates the linear address to an executable page and 1 if it translates to an execute - disable page. (If
    ///     CR0.PG = 0, CR4.PAE = 0, or IA32_EFER.NXE = 0, every linear address is executable and thus this bit will be 0.)
    ///     Otherwise, this bit is undefined.
    // 12 - NMI unblocking due to IRET

    // 63:13 - Reserved

    _DecodeResult res;
    unsigned int decodedInstructionCount;
    _DInst decodedInstruction;
    _DecodedInst result;
    _DecodeType dt;
    _CodeInfo ci;

    DWORD instructionLength;
    static QWORD guestRip;
    static QWORD lastGuestRip = 0;
    PVOID guestRipVa;
    QWORD guestSsAccessRights;
    static QWORD guestOperandMemoryAddress;
    PBYTE pGuestCodeInOurVas;
    static BOOLEAN guestWantsToReadIntoRax = FALSE;

    VmxRead(NULL, VMCS_GUEST_RIP_ENCODING, &guestRip);

    if (lastGuestRip == guestRip)
    {
        goto send_invalid_vendor;
    }

    lastGuestRip = guestRip;

    VmxRead(NULL, VMCS_VM_EXIT_INSTRUCTION_LENGTH_ENCODING, &instructionLength);

    VmxRead(NULL, VMCS_GUEST_SS_ACCESS_RIGHTS_ENCODING, &guestSsAccessRights);

    VmxRead("VMCS_GUEST_PHYSICAL_ADDRESS_FULL_ENCODING", VMCS_GUEST_PHYSICAL_ADDRESS_FULL_ENCODING, &guestOperandMemoryAddress);

    // check if vmcall was made in Real Mode
    if (GET_BITS(guestSsAccessRights, VMCS_SELECTOR_ACCESS_RIGHTS_DB_BIT_INDEX, 1) == 0)
    {
        LOG("Guest is in real mode");
        dt = Decode16Bits;
    }
    else
    {
        QWORD guestMsrEfer;
        QWORD guestCr0;
        QWORD guestCr3;

        VmxRead(NULL, VMCS_GUEST_CR0_ENCODING, &guestCr0);

        if (guestCr0 & CR0_PG)
        {
            VmxRead(NULL, VMCS_GUEST_CR3_ENCODING, &guestCr3);

            guestRip = MmuGetEntryPhysicalAddressFromPageTable(guestRip, guestCr3);
        }

        VmxRead(NULL, VMCS_GUEST_IA32_EFER_FULL_ENCODING, &guestMsrEfer);

        if (GET_BITS(guestMsrEfer, MSR_EFER_LMA_BIT_INDEX, 1))
        {
            LOG("Guest is in 64 bits protected mode");
            dt = Decode64Bits;
        }
        else
        {
            LOG("Guest is in 32 bits protected mode");
            dt = Decode32Bits;
        }
    }

    LOG("Guest rip physical address %X", guestRip);
    guestRipVa = MmuMapToVa(guestRip, instructionLength, NULL, FALSE);
    pGuestCodeInOurVas = MmuAllocVa(PAGE_SIZE);
    memcpy(pGuestCodeInOurVas, guestRipVa, instructionLength);
    MmuUnmapFromVa((QWORD)guestRipVa, instructionLength);

    ci.codeOffset = 0;
    ci.code = pGuestCodeInOurVas;
    ci.codeLen = instructionLength;
    ci.dt = dt;

    res = distorm_decompose(&ci, &decodedInstruction, 1, &decodedInstructionCount);

    LOG("size[0] %X", (QWORD)decodedInstruction.ops[0].size);
    LOG("index[0] %X", (QWORD)decodedInstruction.ops[0].index);
    LOG("size[1] %X", (QWORD)decodedInstruction.ops[1].size);
    LOG("index[1] %X", (QWORD)decodedInstruction.ops[1].index);

    res = distorm_decode(0, pGuestCodeInOurVas, instructionLength, dt, &result, 1, &decodedInstructionCount);
    //LOG("Instruction %s with operands %s", result.mnemonic.p, result.operands.p);

    guestWantsToReadIntoRax = FALSE;
    if (decodedInstruction.ops[0].type == O_REG)
    {
        DWORD index = decodedInstruction.ops[0].index;

        if (decodedInstruction.ops[0].size == 0x20)
        {
            index -= REGS32_BASE;
        }
        else if (decodedInstruction.ops[0].size == 0x10)
        {
            index -= REGS16_BASE;
        }
        else if (decodedInstruction.ops[0].size == 0x40)
        {
            index -= REGS64_BASE;
        }
        else
        {
            index -= REGS8_BASE;
        }

        if (index == 0)
        {
            guestWantsToReadIntoRax = TRUE;
        }
    }

send_invalid_vendor:

    //guestRipOperandMemoryAddress += ProcessorState->RegisterValues[RegisterR9];

    if (guestOperandMemoryAddress == PciGetReservedSerialPortEcamPa() && guestWantsToReadIntoRax)
    {
        ProcessorState->RegisterValues[RegisterRax] = MAX_DWORD;
    }

    VmxJumpToNextInstr();

    return STATUS_SUCCESS;
}

NTSTATUS
VmxBasicExitReasonRdmsr(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    UNREFERENCED_PARAMETER(ProcessorState);

    return STATUS_SUCCESS;
}

NTSTATUS VmxBasicExitReasonVmcall(
    _In_ PPROCESSOR_STATE ProcessorState
)
{
    QWORD ssAccessRights;

    LOG("Exit reason: VMX_BASIC_EXIT_REASON_VMCALL");

    VmxRead(NULL, VMCS_GUEST_SS_ACCESS_RIGHTS_ENCODING, &ssAccessRights);

    if (GET_BITS(ssAccessRights, VMCS_SELECTOR_ACCESS_RIGHTS_DB_BIT_INDEX, 1) == 0)
    {
        if (ProcessorState->RegisterValues[RegisterRax] == INT_15H_E820)
        {
            PQWORD pGuestRflagsFromStackVa;
            QWORD guestRsp;
            QWORD guestRflagsFromStackPa;

            VmxRead(NULL, VMCS_GUEST_RSP_ENCODING, &guestRsp);

            guestRflagsFromStackPa = guestRsp + 4; // since guest is in in real-mode, guestRsp points to guest RIP, guestRsp + 2 points to guest CS and guestRsp + 4 points to rflags

            pGuestRflagsFromStackVa = MmuMapToVa(guestRflagsFromStackPa, PAGE_SIZE, NULL, FALSE);

            CLEAR_BIT(*pGuestRflagsFromStackVa, RFLAGS_CF_BIT_INDEX);

            if (ProcessorState->RegisterValues[RegisterRdx] == INT_15H_E820_SIGNATURE)
            {
                PE820_MEMORY_ENTRY pGuestDestinationBufferVa;
                PE820_MEMORY_ENTRY pCurrentModifiedE820MemoryEntry;
                QWORD guestDestinationBufferPa;
                WORD guestEsSegmentSelector;
                BOOLEAN bLastModifiedE820MemoryEntry;

                VmxRead(NULL, VMCS_GUEST_ES_ENCODING, &guestEsSegmentSelector);

                guestDestinationBufferPa = (guestEsSegmentSelector << 4) + (DWORD)ProcessorState->RegisterValues[RegisterRdi];
                pGuestDestinationBufferVa = (PE820_MEMORY_ENTRY)MmuMapToVa(guestDestinationBufferPa, PAGE_SIZE, NULL, FALSE);


                pCurrentModifiedE820MemoryEntry = MmuGetModifiedE820MemoryEntry((DWORD)ProcessorState->RegisterValues[RegisterRbx], &bLastModifiedE820MemoryEntry);

                if (pCurrentModifiedE820MemoryEntry == NULL)
                {
                    LOG("E820 error");
                    SET_BITT(*pGuestRflagsFromStackVa, RFLAGS_CF_BIT_INDEX);
                }
                else if (ProcessorState->RegisterValues[RegisterRcx] >= 0x14)
                {
                    pGuestDestinationBufferVa->BaseAddress = pCurrentModifiedE820MemoryEntry->BaseAddress;
                    pGuestDestinationBufferVa->Length = pCurrentModifiedE820MemoryEntry->Length;
                    pGuestDestinationBufferVa->Type = pCurrentModifiedE820MemoryEntry->Type;

                    if (bLastModifiedE820MemoryEntry)
                    {
                        ProcessorState->RegisterValues[RegisterRbx] = 0;

                    }
                    else
                    {
                        ProcessorState->RegisterValues[RegisterRbx] = ProcessorState->RegisterValues[RegisterRbx] + 1;
                    }

                    ProcessorState->RegisterValues[RegisterRax] = INT_15H_E820_SIGNATURE;
                }
                else
                {
                    LOG("E820 error");
                    SET_BITT(*pGuestRflagsFromStackVa, RFLAGS_CF_BIT_INDEX);
                }

                MmuUnmapFromVa((QWORD)pGuestDestinationBufferVa, PAGE_SIZE);
            }
            else
            {
                LOG("E820 error");
                SET_BITT(*pGuestRflagsFromStackVa, RFLAGS_CF_BIT_INDEX);
            }

            MmuUnmapFromVa((QWORD)pGuestRflagsFromStackVa, PAGE_SIZE);

            VmxJumpToNextInstr();
        }
        else // original 15h interrupt service
        {
            VMX_OPERATION(VMCS_GUEST_CS_ENCODING,      (QWORD)m_originalInt15hIdtEntry.SegmentSelector,        VMX_WRITE);
            VMX_OPERATION(VMCS_GUEST_CS_BASE_ENCODING, (QWORD)(m_originalInt15hIdtEntry.SegmentSelector << 4), VMX_WRITE);
            VMX_OPERATION(VMCS_GUEST_RIP_ENCODING,     (QWORD)m_originalInt15hIdtEntry.Offset,                 VMX_WRITE);
        }
    }

    //VmxJumpToNextInstr();
    return STATUS_SUCCESS;
}

typedef struct _EXIT_HANDLER
{
    PFUNC_VmxExitHandler  Func;
    BOOLEAN               Status;

}EXIT_HANDLER, *PEXIT_HANDLER;

static
EXIT_HANDLER
gExitHandlerStruct[] =
{
    { (PFUNC_VmxExitHandler)VmxExitHandlerExceptionNmi,                     FALSE }, // 0
    { (PFUNC_VmxExitHandler)VmxExitHandlerExternalInterrupt,                FALSE }, // 1
    { (PFUNC_VmxExitHandler)VmxExitHandlerTripleFault,                      FALSE }, // 2
    { (PFUNC_VmxExitHandler)VmxExitHandlerInitSignal,                       FALSE }, // 3
    { (PFUNC_VmxExitHandler)VmxExitHandlerStartupIpiSipi,                   FALSE }, // 4
    { (PFUNC_VmxExitHandler)VmxExitHandlerIOSystemMananagementInterrupt,    FALSE }, // 5
    { (PFUNC_VmxExitHandler)VmxExitHandlerOtherSmi,                         FALSE }, // 6
    { (PFUNC_VmxExitHandler)VmxExitHandlerInterruptWindow,                  FALSE }, // 7
    { (PFUNC_VmxExitHandler)VmxExitHandlerNmiWindow,                        FALSE }, // 8
    { (PFUNC_VmxExitHandler)VmxExitHandlerTaskSwitch,                       TRUE  }, // 9
    { (PFUNC_VmxExitHandler)VmxExitHandlerCpuid,                            TRUE  }, // 10
    { (PFUNC_VmxExitHandler)VmxExitHandlerGetsec,                           FALSE }, // 11
    { (PFUNC_VmxExitHandler)VmxExitHandlerHlt,                              FALSE }, // 12
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvd,                             FALSE }, // 13
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvlpg,                           FALSE }, // 14
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdpmc,                            FALSE }, // 15
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdtsc,                            FALSE }, // 16
    { (PFUNC_VmxExitHandler)VmxExitHandlerRsm,                              FALSE }, // 17
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmcall,                           TRUE  }, // 18
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmclear,                          FALSE }, // 19
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmlaunch,                         FALSE }, // 20
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmptrld,                          FALSE }, // 21
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmptrst,                          FALSE }, // 22
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmread,                           FALSE }, // 23
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmresume,                         FALSE }, // 24
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxwrite,                         FALSE }, // 25
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxoff,                           FALSE }, // 26
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxon,                            FALSE }, // 27
    { (PFUNC_VmxExitHandler)VmxExitHandlerCRAccess,                         FALSE }, // 28
    { (PFUNC_VmxExitHandler)VmxExitHandlerMovDr,                            FALSE }, // 29
    { (PFUNC_VmxExitHandler)VmxExitHandlerIOInstruction,                    TRUE  }, // 30
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdmsr,                            FALSE }, // 31
    { (PFUNC_VmxExitHandler)VmxExitHandlerWrmsr,                            FALSE }, // 32
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailInvGState,             FALSE }, // 33
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailMsrLoad,               FALSE }, // 34
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 35
    { (PFUNC_VmxExitHandler)VmxExitHandlerMwait,                            FALSE }, // 36
    { (PFUNC_VmxExitHandler)VmxExitHandlerMonitorTrapFlag,                  FALSE }, // 37
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 38
    { (PFUNC_VmxExitHandler)VmxExitHandlerMonitor,                          FALSE }, // 39
    { (PFUNC_VmxExitHandler)VmxExitHandlerPause,                            FALSE }, // 40
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmEntryFailMachineCheckEvent,     FALSE }, // 41
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT // 42
    { (PFUNC_VmxExitHandler)VmxExitHandlerTrpBelowThreshold,                FALSE }, // 43
    { (PFUNC_VmxExitHandler)VmxExitHandlerApicAccess,                       FALSE }, // 44
    { (PFUNC_VmxExitHandler)VmxExitHandlerVirtualizedEoi,                   FALSE }, // 45
    { (PFUNC_VmxExitHandler)VmxExitHandlerAccessGdtrToIdtr,                 FALSE }, // 46
    { (PFUNC_VmxExitHandler)VmxExitHandlerAccessLdtrToTr,                   FALSE }, // 47
    { (PFUNC_VmxExitHandler)VmxExitHandlerEptViolation,                     TRUE  }, // 48
    { (PFUNC_VmxExitHandler)VmxExitHandlerEptMisconfiguration,              FALSE }, // 49
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvept,                           FALSE }, // 50
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdtscp,                           FALSE }, // 51
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmxPreemptionTimerExpired,        TRUE  }, // 52
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvvpid,                          FALSE }, // 53
    { (PFUNC_VmxExitHandler)VmxExitHandlerWbinvd,                           FALSE }, // 54
    { (PFUNC_VmxExitHandler)VmxExitHandlerXsetbv,                           TRUE  }, // 55
    { (PFUNC_VmxExitHandler)VmxExitHandlerApicWrite,                        FALSE }, // 56
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdrand,                           FALSE }, // 57
    { (PFUNC_VmxExitHandler)VmxExitHandlerInvpcid,                          FALSE }, // 58
    { (PFUNC_VmxExitHandler)VmxExitHandlerVmfunc,                           FALSE }, // 59
    { (PFUNC_VmxExitHandler)VmxExitHandlerEncls,                            FALSE }, // 60
    { (PFUNC_VmxExitHandler)VmxExitHandlerRdseed,                           FALSE }, // 61
    { (PFUNC_VmxExitHandler)VmxExitHandlerPageModificationLogFull,          FALSE }, // 62
    { (PFUNC_VmxExitHandler)VmxExitHandlerXSaves,                           FALSE }, // 63
    { (PFUNC_VmxExitHandler)VmxExitHandlerXrstors,                          FALSE }, // 64
    { (PFUNC_VmxExitHandler)VmxExitHandlerDefault,                          FALSE }, // DEFAULT ^65
};

//VOID
//VmxVmExitHandler(
//    PPROCESSOR_STATE ProcessorState
//)
//{
//    //
//    //  Default Vmx Exit Handler
//	//
//    WORD basicExitReason;
//    DWORD exitReason;
//    NTSTATUS ntStatus;
//
//    ntStatus = STATUS_SUCCESS;
//    VmxRead(NULL, VMCS_EXIT_REASON_ENCODING, &exitReason);
//    basicExitReason = exitReason & VMX_BASIC_EXIT_REASON_MASK;
//
//    if (gExitHandlerStruct[basicExitReason].Status != FALSE)
//    {
//        //
//        // Calling the handler for the given exit reason
//        //
//
//        LOG_WARNING("Basic Exit Reason: [0x%08x]", basicExitReason);
//        ntStatus = gExitHandlerStruct[basicExitReason].Func(ProcessorState);
//        if (!NT_SUCCESS(ntStatus))
//        {
//            LOG_ERROR("VmxExitHandler failed, exit code: [0x%08x]", basicExitReason);
//        }
//
//        //RestoreGuestRegistersAndResume(ProcessorState);
//    }
//    else
//    {
//        //VmxExitHandlerDefault(ProcessorState);
//        RestoreGuestRegistersAndResume(ProcessorState);
//    }
//}

VOID
VmxVmExitHandler(
    PPROCESSOR_STATE ProcessorState
)
{
    WORD basicExitReason;
    DWORD exitReason;

    VmxRead(NULL, VMCS_EXIT_REASON_ENCODING, &exitReason);
    basicExitReason = exitReason & VMX_BASIC_EXIT_REASON_MASK;

    //LOG_WARNING("%d", basicExitReason);

    switch (basicExitReason)
    {
    case VMX_BASIC_EXIT_REASON_PREEMPTION_TIMER_EXPIRED:
    {
        VmxBasicExitReasonPreemptionTimerExpired(ProcessorState);
        break;
    }
    case VMX_BASIC_EXIT_REASON_TASK_SWITCH: // 32 bit guest; it wants to restart; hence, we're gonna restart
    {
        VmxBasicExitReasonTaskSwitch(ProcessorState);
        break;
    }
    case VMX_BASIC_EXIT_REASON_IO_INSTRUCTION:
    {
        VmxBasicExitReasonIoInstruction(ProcessorState);
        break;
    }
    case VMX_BASIC_EXIT_REASON_CPUID:
    {
        VmxBasicExitReasonCpuid(ProcessorState);
        break;
    }
    case VMX_BASIC_EXIT_REASON_XSETBV:
    {
        VmxBasicExitReasonXsetbv(ProcessorState);
        break;
    }

    /*case VMX_BASIC_EXIT_REASON_EPT_VIOLATION_SEC:
    {
        VmxBasicExitReasonEptViolation(ProcessorState);
        break;
    }*/
    //case VMX_BASIC_EXIT_REASON_SIPI_SIGNAL:
    //{
    //    QWORD exitQualification;
    //    BYTE apVector;

    //    VmxRead(NULL, VMCS_EXIT_QUALIFICATION_ENCODING, &exitQualification);

    //    apVector = exitQualification & MAX_BYTE;

    //    VMX_OPERATION(NULL, VMCS_GUEST_RIP_ENCODING, 0);

    //    VMX_OPERATION(NULL, VMCS_GUEST_CS_ENCODING, (QWORD)(apVector << 8)); // this selector will be shifted with 4 bits when used in Real Mode

    //    VMX_OPERATION(NULL, VMCS_GUEST_CS_BASE_ENCODING, (QWORD)(apVector << 12));

    //    VMX_OPERATION(NULL, VMCS_GUEST_RSP_ENCODING, (QWORD)0);

    //    VMX_OPERATION(NULL, VMCS_GUEST_ACTIVITY_STATE_ENCODING, Active);

    //    break;
    //}

    case VMX_BASIC_EXIT_REASON_VMCALL:
    {
        VmxBasicExitReasonVmcall(ProcessorState);
        break;
    }
    case VMX_BASIC_EXIT_REASON_EPT_VIOLATION:
    {
        VmxBasicExitReasonEptViolation(ProcessorState);
        break;
    }
    default:
        LOG_ERROR("Undefined basic exit reason %X", (QWORD)basicExitReason);
        LOG_ERROR("Calling vmxoff");

        goto vmxoff;
    }

    RestoreGuestRegistersAndResume(ProcessorState);
    return;

    vmxoff:
    LOG_WARNING("Exiting vmx");
    __vmx_off();
}

VOID
VmxSetVmcsGuestControlRegisters(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_CR0_ENCODING,               0x20,   VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_CR3_ENCODING,               0,      VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_CR4_ENCODING,               0x2000, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_IA32_SYSENTER_EIP_ENCODING, 0,      VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_IA32_SYSENTER_ESP_ENCODING, 0,      VMX_WRITE);

}

VOID
VmxSetVmcsGuestSegmentSelectors(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_ES_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_CS_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_SS_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_DS_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_FS_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_GS_ENCODING,   0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_LDTR_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_TR_ENCODING,   0, VMX_WRITE);
}

VOID
VmxSetVmcsGuestSegmentBaseAddresses(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_CS_BASE_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_FS_BASE_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_GS_BASE_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_TR_BASE_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_SS_BASE_ENCODING, 0, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_DS_BASE_ENCODING, 0, VMX_WRITE);
}

VOID
VmxSetVmcsGuestSegmentLimits(
    VOID
)
{
    DWORD csLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD gsLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD trLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD ssLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD dsLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD esLimit = GDT_REAL_MODE_SELECTOR_LIMIT;
    DWORD fsLimit = GDT_REAL_MODE_SELECTOR_LIMIT;

    VMX_OPERATION(VMCS_GUEST_CS_LIMIT_ENCODING, csLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_TR_LIMIT_ENCODING, trLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_GS_LIMIT_ENCODING, gsLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_DS_LIMIT_ENCODING, dsLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_SS_LIMIT_ENCODING, ssLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_ES_LIMIT_ENCODING, esLimit, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_FS_LIMIT_ENCODING, fsLimit, VMX_WRITE);
}

VOID
VmxSetVmcsGuestSegmentAccessRights(
    VOID
)
{
    LOG_INFO("Setting GUEST SEGMENT access rights");

    DWORD csAccessRights = 0;
    DWORD trAccessRights = 0;
    DWORD ldtrAccessRights = 0;
    DWORD gsAccessRights = 0;
    DWORD fsAccessRights = 0;
    DWORD esAccessRights = 0;
    DWORD dsAccessRights = 0;
    DWORD ssAccessRights = 0;

    csAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(CS_TYPE_EXECUTE_READ_ACCESSED);
    SET_BITT(csAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(csAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_CS_ACCESS_RIGHTS_ENCODING, csAccessRights, VMX_WRITE);


    ssAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(DS_TYPE_READ_WRITE_ACCESSED);
    SET_BITT(ssAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(ssAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_SS_ACCESS_RIGHTS_ENCODING, ssAccessRights, VMX_WRITE);


    dsAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(DS_TYPE_READ_WRITE_ACCESSED);
    SET_BITT(dsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(dsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_DS_ACCESS_RIGHTS_ENCODING, dsAccessRights, VMX_WRITE);
   
    esAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(DS_TYPE_READ_WRITE_ACCESSED);
    SET_BITT(esAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(esAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_ES_ACCESS_RIGHTS_ENCODING, esAccessRights, VMX_WRITE);

    fsAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(DS_TYPE_READ_WRITE_ACCESSED);
    SET_BITT(fsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(fsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_FS_ACCESS_RIGHTS_ENCODING, fsAccessRights, VMX_WRITE);

    gsAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(DS_TYPE_READ_WRITE_ACCESSED);
    SET_BITT(gsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_S_BIT_INDEX);
    SET_BITT(gsAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_GS_ACCESS_RIGHTS_ENCODING, gsAccessRights, VMX_WRITE);

    SET_BITT(ldtrAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_UNUSABLE_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_LDTR_ACCESS_RIGHTS_ENCODING, ldtrAccessRights, VMX_WRITE);

    trAccessRights |= VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_TYPE_MASK(TR_TYPE_32_BIT_BUSY_TSS);
    SET_BITT(trAccessRights, VMX_SEGMEMT_REGISTERS_ACCESS_RIGHTS_P_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_TR_ACCESS_RIGHTS_ENCODING, trAccessRights, VMX_WRITE);
}

VOID
VmxSetVmcsGuestSegmentRegisters(
    VOID
)
{
    VmxSetVmcsGuestSegmentSelectors();
    VmxSetVmcsGuestSegmentBaseAddresses();
    VmxSetVmcsGuestSegmentLimits();
    VmxSetVmcsGuestSegmentAccessRights();
}


VOID
VmxSetVmcsGuestDescriptorTableRegisters(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_GDTR_BASE_ENCODING,  0,        VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_GDTR_LIMIT_ENCODING, MAX_WORD, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_IDTR_BASE_ENCODING,  0,        VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_IDTR_LIMIT_ENCODING, 0x3FF,    VMX_WRITE);
}



VOID
VmxSetVmcsGuestRipRspAndRflags(
    PCPU Cpu
)
{
    QWORD rflags = 0;

    VMX_OPERATION(VMCS_GUEST_RIP_ENCODING, GUEST_MBR_LOADER_LOCATION_PA, VMX_WRITE);
    VMX_OPERATION(VMCS_GUEST_RSP_ENCODING, GUEST_STACK_PA,               VMX_WRITE);

    SET_BITT(rflags, RFLAGS_RESERVED1_BIT_INDEX);
    VMX_OPERATION(VMCS_GUEST_RFLAGS_ENCODING, rflags, VMX_WRITE);
}

VOID
VmxSetVmcsGuestActivityState(
    ACTIVITY_STATE ActivityState
)
{
    VMX_OPERATION(VMCS_GUEST_ACTIVITY_STATE_ENCODING, ActivityState, VMX_WRITE);
}


NTSTATUS
VmxSetVmcsGuestInterruptibilityState()
{ 
    VMX_OPERATION(VMCS_GUEST_INTERRUPTIBILITY_STATE_ENCODING, 0, VMX_WRITE);
	return STATUS_ABANDON_HIBERFILE;
}

VOID
VmxSetVmcsGuestPendingDebugExceptions(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS_ENCODING, 0, VMX_WRITE);
}

VOID
VmxSetVmcsGuestVmcsLinkPointer(
    QWORD VmcsLinkPointer
    )
{
    VMX_OPERATION(VMCS_LINK_POINTER_FULL_ENCODING, VmcsLinkPointer, VMX_WRITE);
}

VOID
VmxSetVmcsGuestVmxPreemptionTimer(
    VOID
)
{
    VMX_OPERATION(VMCS_GUEST_VMX_PREEMPTION_TIMER_ENCODING, VMCS_PREEMPTION_TIMER_DUMMY_VALUE, VMX_WRITE);
}

VOID
VmxSetVmcsIoBitmaps(
    VOID
)
{
    PVOID ioBitmap;

    ioBitmap = MmuAllocVa(VMX_IO_BITMAP_SIZE);
    memzero(ioBitmap, VMX_IO_BITMAP_SIZE);

    VmxSetBitInBitmap(ioBitmap, PCI_CONFIG_ADDRESS_PORT, TRUE);
    //VmxSetBitInBitmap(ioBitmap, PCI_CONFIG_DATA_PORT);
    VMX_OPERATION(VMCS_ADDRESS_OF_IO_BITMAP_A_FULL_ENCODING, (QWORD)VA2PA(ioBitmap), VMX_WRITE);
    MmuUnmapFromVa((QWORD)ioBitmap, VMX_IO_BITMAP_SIZE);

    ioBitmap = MmuAllocVa(VMX_IO_BITMAP_SIZE);
    memzero(ioBitmap, VMX_IO_BITMAP_SIZE);

    VMX_OPERATION(VMCS_ADDRESS_OF_IO_BITMAP_B_FULL_ENCODING, (QWORD)VA2PA(ioBitmap), VMX_WRITE);
    MmuUnmapFromVa((QWORD)ioBitmap, VMX_IO_BITMAP_SIZE);
}


#define ACTIVATE_EXCEPTION_BITMAP 0

VOID
VmxSetVmcsExceptionBitmapControls(
    VOID
)
{
    DWORD exceptionBitmap = 0;

    if (ACTIVATE_EXCEPTION_BITMAP)
    {
        exceptionBitmap = MAX_DWORD;
    }

    VMX_OPERATION(VMCS_EXCEPTION_BITMAP_ENCODING, exceptionBitmap, VMX_WRITE);
}
VOID
VmxSetVmcsGuestStateArea(
    PCPU Cpu,
    ACTIVITY_STATE ActivityState
)
{
    VmxSetVmcsGuestControlRegisters();
    VmxSetVmcsGuestSegmentRegisters();
    VmxSetVmcsGuestDescriptorTableRegisters();
    VmxSetVmcsGuestRipRspAndRflags(Cpu);

    VmxSetVmcsGuestActivityState(ActivityState);
    VmxSetVmcsGuestInterruptibilityState();
    VmxSetVmcsGuestPendingDebugExceptions();
    VmxSetVmcsExceptionBitmapControls();
    VmxSetVmcsIoBitmaps();
    VmxSetVmcsGuestVmcsLinkPointer(VMCS_INVALID_VMCS_LINK_POINTER);
    VmxSetVmcsGuestVmxPreemptionTimer();
}


NTSTATUS
VmxLaunch(
    VOID
)
{
    VMX_STATUS vmxStatus;
    vmxStatus = __vmx_vmlaunch();

    LOG("__vmx_vmlaunch() result: %d", vmxStatus);
    return VmxStatusToNtStatus(vmxStatus);
}


VOID
VmxConfigureGuestCode(
    VOID
)
{
    PVOID guestLocationVa;
    QWORD guestSize;

    guestLocationVa = MmuMapToVa(GUEST_MBR_LOADER_LOCATION_PA, PAGE_SIZE, NULL, FALSE);

	///
    guestSize = (QWORD)_end_guest_16 - (QWORD)_start_guest_16;
    memcpy(guestLocationVa, _start_guest_16, guestSize);
	/// 


    MmuUnmapFromVa((QWORD)guestLocationVa, PAGE_SIZE);
}


PVOID
VmxGetPml4Pa(
    VOID
)
{
    return (PVOID)EPT_PML4_PA;
}


QWORD
GetFailReason(
    VOID
)
{
    QWORD errCode;
    VMX_STATUS vmxStatus = __vmx_vmread(VMCS_VM_INSTRUCTION_ERROR_ENCODING, &errCode);
    NTSTATUS ntStatus = VmxStatusToNtStatus(vmxStatus);

    if (!NT_SUCCESS(ntStatus))
    {
        LOG_ERROR("Couldn't read from VM_INSTRUCTION_ERROR_ENCODING");
    }

    return errCode;
}


VOID
EnableSSE(
    VOID
    )
{
    QWORD cr0;
    QWORD cr4;

    cr0 = __readcr0();
    cr4 = __readcr4();

    CLEAR_BITT(cr0, CR0_EM);
    SET_BITT(cr0, CR0_MP);
    SET_BITT(cr4, CR4_OSFXSR);
    SET_BITT(cr4, CR4_OSXMMEXCPT);
    __writecr0(cr0);
    __writecr4(cr4);

    LOG_INFO("CR0.EM cleared, CR0.MP set, CR4.OSFXSR set, CR4.OSXMMECPT set");
}

NTSTATUS 
InitializeFeatures(
    VOID
)
{
    uintptr_t featCtrl;
    const QWORD neededFeatMask = MSR_IA32_FEATURE_CONTROL_LOCK_BIT | MSR_IA32_FEATURE_CONTROL_VMXON_OUTSIDE_SMX;
    
    featCtrl = __readmsr(MSR_IA32_FEATURE_CONTROL);
    if ((featCtrl & neededFeatMask) == neededFeatMask)
    {
        return STATUS_SUCCESS;
    }

    // Atempt to write the required features
    __writemsr(MSR_IA32_FEATURE_CONTROL, featCtrl | neededFeatMask);
    if ((featCtrl & neededFeatMask) == neededFeatMask)
    {
        return STATUS_SUCCESS;
    }

    return STATUS_MSR_FEATURE_CONTROL_REQ_BITS_FAIL;
}


NTSTATUS
VmxInit(
    _In_ PCPU Cpu
    )
{
    NTSTATUS ntStatus;

    // PVCPU structure
    PVCPU pvCpu;

    // Discovering VMX support (23.6)
    LOG_INFO("Checking for VMX support");
    ntStatus = CheckVmxSupported();
    if (!NT_SUCCESS(ntStatus))
    {
        LOG_ERROR("VMX not supported: [0x%08x]", ntStatus);
        return ntStatus;
    }

    // Enable VMX by setting bit 13 of CR4 (23.7)
    Cr4SetBit(CR4_VMXE_BIT_ENABLE);

    // Checking for bit OXSAVE (Enable AVX)
    if (CpuidGetFeature(0x01, CPUID_RCX, CPUID_01H_RCX_XSAVE_BIT_INDEX, 1) && CpuidGetFeature(0x01, CPUID_RDX, CPUID_01_RDX_SSE_SUPPORTED, 1))
    {
        Cr4SetBit(CR4_OSXSAVE_BIT_INDEX);
    }
    LOG_INFO("Bit 13 of CR4 set");

    LOG_INFO("Enabling [SSE]");
    EnableSSE();
    LOG_INFO("[SSE] Successfully enabled");


    // Allocate the VMCS region | 4-KByte naturally aligned (24.1)
    Cpu->VmxonRegionVa = MmuAllocVa(PAGE_SIZE);
    LOG_INFO("Successfully allocated VMCS region");


    LOG_MSR("Check for: MSR_IAMhvVmxEntryHandler32_FEATURE_CONTROL (MSR_IA32_FEATURE_CONTROL) [0x%08x]\n" 
            "Bit: (MSR_IA32_FEATURE_CONTROL_LOCK_BIT) [0x%08x]\n"
            "Mask: (BIT_MASK) [0x%08x]", 
            MSR_IA32_FEATURE_CONTROL, MSR_IA32_FEATURE_CONTROL_LOCK_BIT, BIT_MASK);


    if (!ReadMsr(MSR_IA32_FEATURE_CONTROL, MSR_IA32_FEATURE_CONTROL_LOCK_BIT, BIT_MASK))
    {
        MsrSetBit(MSR_IA32_FEATURE_CONTROL, MSR_IA32_FEATURE_CONTROL_VMXON_OUTSIDE_SMX);
        MsrSetBit(MSR_IA32_FEATURE_CONTROL, MSR_IA32_FEATURE_CONTROL_LOCK_BIT);
    }

    LOG_MSR("MSR_IA32_FEATURE_CONTROL: 0x%X", ReadMsr(MSR_IA32_FEATURE_CONTROL, 0, MAX_QWORD));
    LOG_MSR("MSR_IA32_BASIC_VMX: 0x%X", ReadFullMsr(MSR_IA32_VMX_BASIC));

    VmxSetRestrictedValues();

    LOG_INFO("Setting revision identifier");
    VmxSetRevisionIndentifier((PVMCS)Cpu->VmxonRegionVa);

    Cpu->VmxonRegionPa = (PVOID)VA2PA(Cpu->VmxonRegionVa);
    ntStatus = VmxOn(&Cpu->VmxonRegionPa);
    if (!NT_SUCCESS(ntStatus))
    {
        LOG("VmxOn failed");
    }
    else
    {
        LOG("VMX NT_SUCCESS");
    }

    Cpu->Vcpu = MmuAllocVa(PAGE_SIZE);
    pvCpu = (PVCPU)Cpu->Vcpu;

    pvCpu->Vmcs = MmuAllocVa(VMX_VXMON_REGION_SIZE);
    memzero((PBYTE)pvCpu->Vmcs, VMX_VXMON_REGION_SIZE);

    VmxSetRevisionIndentifier((PVMCS)pvCpu->Vmcs);

    pvCpu->VmcsPa = (PVOID)VA2PA(pvCpu->Vmcs);

    ntStatus = VmxClear(&pvCpu->VmcsPa);
    if (!NT_SUCCESS(ntStatus))
    {
        LOG_ERROR("VmxClear failed");
        LOG_WARN_HLT("");
    }
    else
    {
        LOG_INFO("VmxClear NT_SUCCESS");
    }

    QWORD currentVmcsPointer;
    __vmx_vmptrst(&currentVmcsPointer);

    if (currentVmcsPointer != MAX_QWORD)
    {
        LOG_ERROR("Current Vmcs pointer should be 0xFF ... FF (MAX QWORD), got value: %X%X", currentVmcsPointer >> 32, currentVmcsPointer);
        LOG_WARN_HLT("");
    }

    /// 30.3
    LOG_INFO("Load current VMCS pointer from memory");
    ntStatus = VmxPtrld(&pvCpu->VmcsPa);
    if (!NT_SUCCESS(ntStatus))
    {
        LOG_ERROR("VmxPtrld failed");
        LOG_WARN_HLT("");
    }
    else
    {
        LOG_INFO("VmxPtrld NT_SUCCESS");
    }


    VmxSetVmcsVmxControls();
    //DetermineN();

    VmxSetVmcsHostArea(Cpu);


    LOG_INFO("Going to mark BSP in active state, APs will be marked in WaitForSipi state");

    if ((*Cpu).Bsp)
    {
        LOG_INFO("Active CPU [BSP]");
        VmxSetVmcsGuestStateArea(Cpu, Active);
    }
    else /**/
    {
        LOG_INFO("Setting AP to WaitForSipi");
        VmxSetVmcsGuestStateArea(Cpu, WaitForSipi);
    }


    LOG_INFO("Incremeneting the number of CPUs in VMX Operation");
    IncrementNumberOfCpusInVmxMode();


    LOG_INFO("Calling VmxLaunch...");
    VmxLaunch();

    DecrementNumberOfCpusInVmxMode();

    LOG_ERROR("This part shouldn't be reached");

    LOG_ERROR("Error code received: [0x%08X]", GetFailReason());
    return STATUS_SUCCESS;
}

VOID
VmxPatchRealModeInt15hE820(
    VOID
)
{
    PIDT_ENTRY_REAL_MODE pIdtEntryRealModeVa = MmuMapToVa(REAL_MODE_IDT_BASE, PAGE_SIZE, NULL, FALSE);
    PBYTE pHookedInt15hCodeVa;
    QWORD hookedInt15hCodeSize;

    LOG("Hooking INT15h software interrupt");

    // save original 15h entry
    m_originalInt15hIdtEntry = pIdtEntryRealModeVa[VECTOR_15H];

    // patch 'int 15h' to jump to our code
    memzero((PBYTE)&pIdtEntryRealModeVa[VECTOR_15H], sizeof(IDT_ENTRY_REAL_MODE));
    pIdtEntryRealModeVa[VECTOR_15H].Offset = (WORD)GUEST_MBR_LOADER_SIZE;


    //
    pIdtEntryRealModeVa[VECTOR_15H].SegmentSelector = (WORD)(GUEST_MBR_LOADER_LOCATION_PA >> 4);

    //unmap IDT
    MmuUnmapFromVa((QWORD)pIdtEntryRealModeVa, PAGE_SIZE);

    // copy our 'int 15h' code into the first 1MB area
    pHookedInt15hCodeVa = MmuMapToVa(PATCHED_INT15H_OFFSET, PAGE_SIZE, NULL, FALSE);

    hookedInt15hCodeSize = (QWORD)_end_hooked_int15h - (QWORD)_start_hooked_int15h;
#pragma warning(suppress:4152)
    memcpy(pHookedInt15hCodeVa, _start_hooked_int15h, hookedInt15hCodeSize);
    MmuUnmapFromVa((QWORD)pHookedInt15hCodeVa, PAGE_SIZE);

    // create E820 modified map
    MmuCreateModifiedE820MemoryMap();

    // log modified memory map
    MmuPrintModifiedE820MemoryEntries();
}
