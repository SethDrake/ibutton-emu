#include "onewire.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"


OneWire::OneWire()
{
	usart = nullptr;
}

OneWire::~OneWire()
{
}

void OneWire::init(USART_TypeDef* usart)
{
	this->usart = usart;
	if (usart == USART1)
	{
		dmaRxChannel = DMA1_Channel5;	
		dmaTxChannel = DMA1_Channel4;
		dmaTCFlag = DMA1_FLAG_TC5;
	}
	else if (usart == USART2)
	{
		dmaRxChannel = DMA1_Channel6;	
		dmaTxChannel = DMA1_Channel7;
		dmaTCFlag = DMA1_FLAG_TC6;
	}
}


void OneWire::initDMA()
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(dmaTxChannel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(usart->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = sizeof(buf);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmaTxChannel, &DMA_InitStructure);

	DMA_DeInit(dmaRxChannel);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(usart->DR);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) &buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = sizeof(buf);
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(dmaRxChannel, &DMA_InitStructure);
}

bool OneWire::reset()
{
	USART_Cmd(usart, DISABLE);
	setUsartBaudrate(9600);
	USART_Cmd(usart, ENABLE);
	USART_ClearFlag(usart, USART_FLAG_TC);
	USART_SendData(usart, 0xF0);
	while (USART_GetFlagStatus(usart, USART_FLAG_TC) == RESET) {
#ifdef RTOS_DELAY_FUNC
		RTOS_DELAY_FUNC;
#endif
	}
	const uint8_t response = USART_ReceiveData(usart);
	USART_Cmd(usart, DISABLE);
	setUsartBaudrate(115200);
	USART_Cmd(usart, ENABLE);
	return (response != 0xF0);
}

void OneWire::send(uint8_t* command, uint8_t cmdLen, uint8_t* data, uint8_t dataLen, uint8_t dataOffset)
{
	while (cmdLen > 0) {
		byteToSlots(*command, buf);
		command++;
		cmdLen--;

		initDMA();

		USART_ClearFlag(usart, USART_FLAG_RXNE | USART_FLAG_TC | USART_FLAG_TXE);
		USART_DMACmd(usart, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
		DMA_Cmd(dmaRxChannel, ENABLE);
		DMA_Cmd(dmaTxChannel, ENABLE);

		while (DMA_GetFlagStatus(dmaTCFlag) == RESET) {
#ifdef RTOS_DELAY_FUNC
			RTOS_DELAY_FUNC;
#endif
		}

		DMA_Cmd(dmaTxChannel, DISABLE);
		DMA_Cmd(dmaRxChannel, DISABLE);
		USART_DMACmd(usart, USART_DMAReq_Tx | USART_DMAReq_Rx, DISABLE);

		if (dataLen > 0)
		{
			if (dataOffset == 0) {
				*data = slotsToByte(buf);
				data++;
				dataLen--;
			}
			else {
				dataOffset--;
			}	
		}
	}
}

uint8_t OneWire::crc(uint8_t* data, const uint8_t len)
{
	uint8_t crc = 0;
	for (uint8_t i = 0; i < len; i++) 
	{
		uint8_t inbyte = data[i];
		for (uint8_t j = 0; j < 8; j++) 
		{
			const uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
			{
				crc ^= 0x8C;	
			}                  
			inbyte >>= 1;
		}
	}
	return crc;
}

void OneWire::byteToSlots(uint8_t byte, uint8_t* slots)
{
	for (uint8_t i = 0; i < 8; i++) {
		if (byte & 0x01) {
			*slots = 0xFF;
		}
		else {
			*slots = 0x00;
		}
		slots++;
		byte >>= 1;
	}
}

uint8_t OneWire::slotsToByte(uint8_t* slots)
{
	uint8_t byte = 0x00;
	for (uint8_t i = 0; i < 8; i++) {
		byte >>= 1;
		if (*slots == 0xFF) {
			byte |= 0x80;
		}
		slots++;
	}
	return byte;
}

void OneWire::setUsartBaudrate(uint16_t baudrate)
{
	uint32_t apbclock;
	const uint32_t usartxbase = (uint32_t)usart; 
	RCC_ClocksTypeDef RCC_ClocksStatus;
	
	USART_Cmd(this->usart, DISABLE);
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	if (usartxbase == USART1_BASE)
	{
		apbclock = RCC_ClocksStatus.PCLK2_Frequency;
	}
	else
	{
		apbclock = RCC_ClocksStatus.PCLK1_Frequency;
	}
	usart->BRR = (uint16_t)(apbclock / baudrate);
}

