
extern VmxVmExitHandler
extern RestoreRegisters
extern VmxBasicExitReasonVmcall
extern MhvVmxEntryHandler

_GuestEntryRip:

    ; In case of VM EXIT
    ; Set RIP to this addr

    save_proc_state

	mov rcx, rsp

	sub rsp, 0x20

	call MhvVmxEntryHandler

    ; HLT Instructions/sec - Number of CPU halts per seconds on the VP (VIRTUAL PROCESSOR)
    ; A HLT will cause the hypervisor scheduler to de-schedule the current VP and move the the next VP in the runlist
	hlt
