#include "mm.h"

.section ".text.boot"
.equ STACK_SIZE, 4096 //EQUivalent(define)
.globl _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0x3		// Check processor id
	mov x19, x0  
	cbz	x0, master		// Compare and Branch if Zero
	b   secondary

proc_hang: 
	b 	proc_hang

master:
	// stack
	mov	sp, #LOW_MEMORY 
	// clear bss
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero
	// ------------------------------------------------------------
    // QEMU raspi3b: release secondary cores from holding pen
    // Assumption: spin-table at 0xD8, slot = 0xD8 + core_id*8
    // ------------------------------------------------------------
#ifdef QEMU
    adr  x6, _start          // x6 = entry address (your real kernel entry)
    mov  x7, #0xD8           // x7 = spin-table base (hypothesis from QEMU regs)

    str  x6, [x7, #8]        // slot for core1
    str  x6, [x7, #16]       // slot for core2
    str  x6, [x7, #24]       // slot for core3

    dsb  sy                  // ensure stores are globally visible before wakeup
    sev                      // wake secondary cores waiting in WFE
#endif
	// set boot_ready = 1 , secondary go
	adrp x2, boot_ready // which page
	add  x2, x2, :lo12:boot_ready //get boot_ready absolute addr
	mov  w3, #1
	str  w3, [x2]
	dmb  ish // data mem barrier : inner shareable
			 // Ensure sychronization
	sev // send event
	// per-core stack
	bl  set_core_stack
	// goto C laanguage
	mov x0, x19
	bl	kernel_main
	b 	proc_hang		// should never come here

secondary:
	// boot_ready lock(supervise)
	adrp x2, boot_ready
	add x2, x2, :lo12:boot_ready

1: 	ldr  w3, [x2]
	cbnz w3, 2f
	wfe // wait for event
	b 		 1b 	

2:  bl set_core_stack // set the coreX stack
	mov x0, x19
	bl kernel_main
	b proc_hang
// ------------------------------------------------------------
// set_core_stack:
//   independent stack：
//   sp = boot_stacks + core_id*STACK_SIZE + STACK_SIZE
// ------------------------------------------------------------
set_core_stack:
    adrp x4, boot_stacks // Find boot_stack base
    add  x4, x4, :lo12:boot_stacks // get boot_stack absolute addt
    add  x4, x4, x19, lsl #12      // offset core_id * 4096
    add  sp, x4, #STACK_SIZE       // sp: top of the stack
    ret
.section ".data"
.align 2
.globl boot_ready
boot_ready:
    .word 0
// ------------------------------------------------------------
// per-core stacks in .bss (cleared by memzero)
// ------------------------------------------------------------
.section ".bss"
.align 12                          // 4096 對齊
.globl boot_stacks
boot_stacks:
    .skip STACK_SIZE * 4