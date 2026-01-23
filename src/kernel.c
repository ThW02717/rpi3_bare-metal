#include "uart.h"

void kernel_main(void)
{
	uart_init();
    for(int i = 0; i < 5000000; i++) { 
        asm volatile("nop"); 
    }
	uart_send_string("Hello, world!\r\n");

	while (1) {
		uart_send(uart_recv());
	}
}
