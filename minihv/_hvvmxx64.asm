  
  
%include "_utils.mac"
   .code

    extern VmxVmExitHandler:proc
    global _mhv_entry


_mhv_entry:

    save_proc_state

	mov rcx, rsp
	sub rsp, 0x20                     
    call VmxVmExitHandler 
                               
                               
    hlt

    end
