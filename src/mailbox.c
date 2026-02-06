#include "mailbox.h"
#include "peripherals/mailbox.h"
#include "utils.h"

#define MBOX_CHANNEL_PROP 8u

#define REQUEST_CODE     0x00000000u
#define REQUEST_SUCCEED  0x80000000u
#define REQUEST_FAILED   0x80000001u
#define TAG_REQUEST_CODE 0x00000000u
#define END_TAG          0x00000000u

#define GET_BOARD_REVISION 0x00010002u
#define GET_ARM_MEMORY     0x00010005u

// send an addr in RAM to mailbox, mailbox will notify firmware
// MAILBOX_WRITE: send request to firmware
// MAILBOX_READ: recv firmware token
// MAILBOX_STATUS: status register FIFO
unsigned int mailbox_call(unsigned int channel, unsigned int *message)
{
	// RAM message array
	unsigned int addr = (unsigned int)(unsigned long)message;

	unsigned int value = (addr & ~0xFu) | (channel & 0xFu);

	// Wait until mailbox can accept a write.
	while (get32(MAILBOX_STATUS) & MAILBOX_FULL) {
		asm volatile("nop");
	}
	put32(MAILBOX_WRITE, value);
	// write token into mailbox
	while (1) {
		// Wait for a response.
		while (get32(MAILBOX_STATUS) & MAILBOX_EMPTY) {
			asm volatile("nop");
		}
		// response is the request
		unsigned int resp = get32(MAILBOX_READ);
		if (resp == value) {
			return message[1] == REQUEST_SUCCEED;
		}
	}
}

unsigned int get_board_revision(void)
{
	// Beacause we 32-bit token is [31:4] message buffer address
	
	__attribute__((aligned(16))) unsigned int mailbox[7];

	mailbox[0] = 7 * 4;         // buffer size in bytes
	mailbox[1] = REQUEST_CODE;
	mailbox[2] = GET_BOARD_REVISION;
	mailbox[3] = 4;             // value buffer length
	mailbox[4] = TAG_REQUEST_CODE;
	mailbox[5] = 0;
	mailbox[6] = END_TAG;

	if (!mailbox_call(MBOX_CHANNEL_PROP, mailbox)) {
		return 0;
	}
	return mailbox[5];
}

unsigned int get_arm_memory(unsigned int *base, unsigned int *size)
{
	__attribute__((aligned(16))) unsigned int mailbox[8];

	mailbox[0] = 8 * 4;         // buffer size in bytes
	mailbox[1] = REQUEST_CODE;
	mailbox[2] = GET_ARM_MEMORY;
	mailbox[3] = 8;             // value buffer length (base + size)
	mailbox[4] = TAG_REQUEST_CODE;
	mailbox[5] = 0;             // base
	mailbox[6] = 0;             // size
	mailbox[7] = END_TAG;

	if (!mailbox_call(MBOX_CHANNEL_PROP, mailbox)) {
		return 0;
	}

	if (base) {
		*base = mailbox[5];
	}
	if (size) {
		*size = mailbox[6];
	}
	return 1;
}
