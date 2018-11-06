#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#define F_CPU 8000000UL

#include "stm8s.h"
#include "delay.h"

#define LED_PIN  GPIO_PIN_5
#define LED_PORT GPIOB

#define OW_PORT    GPIOC
#define OW_PIN_RX  GPIO_PIN_5
#define OW_PIN_TX  GPIO_PIN_6

#define RESET_WIDTH 480000 

typedef enum
{ 
	IDLE = 0,
	RESET_CMD,
	PRESENCE,
	READ_COMMAND,
	READ_ROM,
	SKIP_ROM,
	MATCH_ROM,
	SEARCH_ROM
} SYSTEM_STATES;

extern SYSTEM_STATES current_state;

static uint8_t getLineState(void);
static void setLineState(uint8_t v);
extern void processEvent(void);

#endif /* __ONEWIRE_H */