#ifndef __UART_H
#define __UART_H



#include <util/setbaud.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <inttypes.h>
#include <stdio.h>
#include <types.h>
#include "../asyncTypes.h"



typedef void (*uart_rx_callback_t)(char);
typedef void (*uart_tx_callback_t)(void);
typedef void (*uart_asyncCallback_t)(void);
typedef char (*uart_readCallback_t)(void);


void uart_putstr(const char* pstr);
void uart_putstrM(const char* pstr);
void uart_putstrAM(const char * pstr, uart_asyncCallback_t callBack);

char * uart_getstr(char * pstr, uint16_t max);

void uartBeginReceive(uart_rx_callback_t callBack);
void uartEndReceive(void);

void uartBeginSend(uart_tx_callback_t callBack);
void uartEndTransmit(void);

void uartWrite(char x);
uint8_t uartDataWaiting(void);
char uartRead(void);

void uartSendBuff(const char * buffer, uint16_t length);
void uartReceiveBuff(char * buffer, uint16_t length);

void uart_init(void);

#endif
