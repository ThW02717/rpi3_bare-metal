#include "uart.h"
#include "mailbox.h"

#ifndef NUM_CORES
#define NUM_CORES 4
#endif

static volatile unsigned int uart_ready = 0;
static volatile unsigned int turn = 0;

static void delay(unsigned int cnt)
{
	while (cnt--) {
		asm volatile("nop");
	}
}

static int is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static unsigned int str_len(const char *s)
{
	unsigned int n = 0;
	while (s[n] != '\0') {
		n++;
	}
	return n;
}

static void trim_in_place(char *s)
{
	char *p = s;
	while (is_space(*p)) {
		p++;
	}
	if (p != s) {
		char *dst = s;
		while (*p != '\0') {
			*dst++ = *p++;
		}
		*dst = '\0';
	}

	unsigned int len = str_len(s);
	while (len > 0 && is_space(s[len - 1])) {
		s[len - 1] = '\0';
		len--;
	}
}

static int streq(const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0') {
		if (*a != *b) {
			return 0;
		}
		a++;
		b++;
	}
	return *a == *b;
}

static void shell_run(void)
{
	char buf[64];
	int ignore_next_lf = 0;

	while (1) {
		uart_send_string("> ");
		unsigned int idx = 0;

		while (1) {
			char c = uart_recv();

			if (ignore_next_lf) {
				if (c == '\n') {
					ignore_next_lf = 0;
					continue;
				}
				ignore_next_lf = 0;
			}

			if (c == '\r' || c == '\n') {
				uart_send_string("\r\n");
				if (c == '\r') {
					ignore_next_lf = 1;
				}
				buf[idx] = '\0';
				break;
			}

			if (c == '\b' || c == 0x7f) {
				if (idx > 0) {
					idx--;
					uart_send_string("\b \b");
				}
				continue;
			}

			if (c >= ' ' && c <= '~') {
				if (idx < sizeof(buf) - 1) {
					buf[idx++] = c;
					uart_send(c);
				}
			}
		}

		trim_in_place(buf);
		if (buf[0] == '\0') {
			continue;
		}

		if (streq(buf, "help")) {
			uart_send_string("help  : print all available commands\r\n");
			uart_send_string("hello : print Hello World!\r\n");
			uart_send_string("reboot:the device\r\n");
			uart_send_string("info  : print board and memory info\r\n");
		} else if (streq(buf, "hello")) {
			uart_send_string("Hello World!\r\n");
		} else if (streq(buf, "info")) {
			unsigned int arm_base = 0;
			unsigned int arm_size = 0;
			uart_send_string("Board revision: 0x");
			uart_send_hex(get_board_revision());
			uart_send_string("\r\n");
			if (get_arm_memory(&arm_base, &arm_size)) {
				uart_send_string("ARM mem base : 0x");
				uart_send_hex(arm_base);
				uart_send_string("\r\n");
				uart_send_string("ARM mem size : 0x");
				uart_send_hex(arm_size);
				uart_send_string(" (");
				uart_send_dec(arm_size / (1024u * 1024u));
				uart_send_string(" MB)\r\n");
			}
		} else if (streq(buf, "reboot")) {
			uart_send_string("reboot the device\r\n");
		}else {
			uart_send_string("Unknown command: ");
			uart_send_string(buf);
			uart_send_string("\r\n");
		}
	}
}

// core_id in x0
void kernel_main(unsigned long core_id)
{
	// core_00 doing Uart init 
	if (core_id == 0) {
		uart_init();
		delay(5000000);
		uart_send_string("Board revision: 0x");
		uart_send_hex(get_board_revision());
		uart_send_string("\r\n");
		unsigned int arm_base = 0;
		unsigned int arm_size = 0;
		if (get_arm_memory(&arm_base, &arm_size)) {
			uart_send_string("ARM mem base : 0x");
			uart_send_hex(arm_base);
			uart_send_string("\r\n");
			uart_send_string("ARM mem size : 0x");
			uart_send_hex(arm_size);
			uart_send_string(" (");
			uart_send_dec(arm_size / (1024u * 1024u));
			uart_send_string(" MB)\r\n");
		}
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
		// if other core not finish, wait
		while (turn < NUM_CORES) {
			asm volatile("nop");
		}
		shell_run();
	}
	// other cores park here in echo stage
    while (1) {
        asm volatile("wfe");
    }
}
