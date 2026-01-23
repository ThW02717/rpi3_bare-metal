#include "uart.h"
static volatile unsigned int uart_ready = 0;
static volatile unsigned int turn = 0;

static void delay(unsigned int cnt)
{
	while (cnt--) {
		asm volatile("nop");
	}
}
// core_id in x0
void kernel_main(unsigned long core_id)
{
	// core_00 doing Uart init 
	if (core_id == 0) {
		uart_init();
		delay(5000000);
		uart_ready = 1;
	} else {
		while (uart_ready == 0) {
			asm volatile("nop");
		}
	}
	// Print message in order
	while (turn != core_id) {
		asm volatile("nop");
	}
	uart_send_string("Hello, from processor ");
    uart_send((char)('0' + core_id));
    uart_send_string("\r\n");
	turn++;
	// Echo only for core0
	if (core_id == 0) {
		while(1) {
			uart_send(uart_recv());
		}
	}
	// other cores park here in echo stage
    while (1) {
        asm volatile("wfe");
    }
}
