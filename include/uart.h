#ifndef	_UART_H
#define	_UART_H

void uart_init ( void );
char uart_recv ( void );
void uart_send ( char c );
void uart_send_string(const char* str);
void uart_send_hex(unsigned int value);
void uart_send_dec(unsigned int value);

#endif  /*_UART_H */
