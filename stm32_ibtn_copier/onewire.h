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
	void skip();
	bool reset();
	uint8_t progImpulse();
	void send(uint8_t *command, uint8_t cmdLen, uint8_t *data, uint8_t dataLen, uint8_t dataOffset);
	void sendSlot(uint8_t slot);
	void setSlowMode(uint8_t enabled);
	uint8_t crc(uint8_t *data, uint8_t len);
protected:
	void byteToSlots(uint8_t byte, uint8_t *slots);
	uint8_t slotsToByte(uint8_t *slots);
	void setUsartBaudrate(const uint32_t baudrate);
	void initDMA(uint8_t bufLen);
private:
	USART_TypeDef* usart;
	DMA_Channel_TypeDef* dmaRxChannel;
	DMA_Channel_TypeDef* dmaTxChannel;
	uint32_t dmaTCRxFlag;
	uint32_t dmaTCTxFlag;
	uint8_t buf[8];
	uint8_t slowMode;
};

#endif /* ONEWIRE_H_ */
