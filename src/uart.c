#include "utils.h"
#include "peripherals/uart.h"
#include "peripherals/gpio.h"

#define UART_CLOCK_HZ 48000000u
#define BAUD_RATE     115200u

static void uart_calc_dividers(unsigned int baud, unsigned int *ibrd, unsigned int *fbrd)
{
	if (baud == 0) {
		*ibrd = 0;
		*fbrd = 0;
		return;
	}

	unsigned long long dividend = (unsigned long long)UART_CLOCK_HZ;
	unsigned long long divisor = (unsigned long long)baud * 16ull;
	unsigned long long integer = dividend / divisor;
	unsigned long long remainder = dividend % divisor;

	if (integer == 0) integer = 1;
	if (integer > 0xFFFFull) integer = 0xFFFFull;

	// Fraction uses 6 bits; round to the nearest representable value.
	unsigned long long fraction = ((remainder * 64ull) + divisor / 2ull) / divisor;
	if (fraction > 0x3Full) fraction = 0x3Full;

	*ibrd = (unsigned int)integer;
	*fbrd = (unsigned int)fraction;
}

void uart_init ( void )
{
	unsigned int selector;
	unsigned int ibrd, fbrd;

	put32(UART0_CR, 0);                     // Disable UART before configuration

	selector = get32(GPFSEL1);
	selector &= ~((7<<12) | (7<<15));
	selector |= (4<<12) | (4<<15);          // ALT0 selects UART0 for GPIO14/15
	put32(GPFSEL1, selector);

	put32(GPPUD, 0);
	delay(150);
	put32(GPPUDCLK0, (1<<14) | (1<<15));
	delay(150);
	put32(GPPUDCLK0, 0);

	put32(UART0_ICR, 0x7FF);                // Clear all pending interrupts

	uart_calc_dividers(BAUD_RATE, &ibrd, &fbrd);
	put32(UART0_IBRD, ibrd);
	put32(UART0_FBRD, fbrd);

	put32(UART0_LCRH, (1<<4) | (3<<5));     // FIFO on, 8-bit words
	put32(UART0_IMSC, 0);                   // Mask and disable UART interrupts
	put32(UART0_CR, (1<<0) | (1<<8) | (1<<9));
}

void uart_send ( char c )
{
	while (get32(UART0_FR) & (1<<5)) {
		// wait for space in the FIFO
	}
	put32(UART0_DR, c);
}

char uart_recv ( void )
{
	while (get32(UART0_FR) & (1<<4)) {
		// wait for data
	}
	return (char)(get32(UART0_DR) & 0xFF);
}

void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}
