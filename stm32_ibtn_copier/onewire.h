#ifndef ONEWIRE_H_
#define ONEWIRE_H_

#include <stdint.h>
#include "stm32f10x_usart.h"

//#define RTOS_DELAY_FUNC osDelay(1)

class OneWire {
public:
	OneWire();
	~OneWire();
	void init(USART_TypeDef* usart);
	bool reset();
	void send(uint8_t *command, uint8_t cmdLen, uint8_t *data, uint8_t dataLen, uint8_t dataOffset);
	uint8_t crc(uint8_t *data, uint8_t len);
protected:
	void byteToSlots(uint8_t byte, uint8_t *slots);
	uint8_t slotsToByte(uint8_t *slots);
	void setUsartBaudrate(uint16_t baudrate);
	void initDMA();
private:
	USART_TypeDef* usart;
	DMA_Channel_TypeDef* dmaRxChannel;
	DMA_Channel_TypeDef* dmaTxChannel;
	uint32_t dmaTCFlag;
	uint8_t buf[8];
};

//#define OW_GIVE_TICK_RTOS

#endif /* ONEWIRE_H_ */
