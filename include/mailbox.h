#ifndef _MAILBOX_H
#define _MAILBOX_H

unsigned int mailbox_call(unsigned int channel, unsigned int *message);
unsigned int get_board_revision(void);
unsigned int get_arm_memory(unsigned int *base, unsigned int *size);

#endif  /* _MAILBOX_H */
