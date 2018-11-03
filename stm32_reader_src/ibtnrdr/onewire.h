/*
 * onewire.h
 *
 *  Version 1.0.1
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// выбираем, на каком USART находится 1-wire
//#define OW_USART1
#define OW_USART2
//#define OW_USART3
//#define OW_USART4


// если нужно отдавать тики FreeRTOS, то раскомментировать
//#define OW_GIVE_TICK_RTOS

// первый параметр функции OW_Send
#define OW_SEND_RESET		1
#define OW_NO_RESET		2

// статус возврата функций
#define OW_OK			1
#define OW_ERROR		2
#define OW_NO_DEVICE	3

#define OW_NO_READ		0xff

#define OW_READ_SLOT	0xff

extern uint8_t OW_Reset(void);
extern uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);
extern uint8_t OW_CRC(uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* ONEWIRE_H_ */
