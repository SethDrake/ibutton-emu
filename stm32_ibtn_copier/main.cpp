#include <main.h>
#include <stm32f10x.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dbgmcu.h"
#include "misc.h"
#include "delay.h"
#include "onewire.h"

OneWire ibutton;
uint8_t ibuttonSn[6];

volatile MODE mode = MODE_READ;

void Debug_Configuration();
void Clock_Config();
void GPIO_Config();
void GPIO_Config();
void USART_Config();
void NVIC_Config();

bool iButton_ReadSN(uint8_t* sn);
bool iButton_WriteSN(uint8_t* sn);
IBUTTON_KEY_TYPE iButton_Test();
void readKeys();
void clearSn();

void Debug_Configuration()
{
	DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STANDBY | DBGMCU_STOP, ENABLE); //Enable debug in powersafe modes
	
	SCB->CCR |= SCB_CCR_DIV_0_TRP;	
}

void Clock_Config()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(SystemCoreClock / 1000);
}

void GPIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = BLUE_LED_PIN | GREEN_LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = ONEWIRE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ONEWIRE_PORT, &GPIO_InitStructure);
}

void USART_Config() {
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(ONEWIRE_USART, &USART_InitStructure);
	USART_HalfDuplexCmd(ONEWIRE_USART, ENABLE);
	USART_Cmd(ONEWIRE_USART, ENABLE);
}

void NVIC_Config()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	/*NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x05;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);*/
}


bool iButton_ReadSN(uint8_t* sn)
{
	uint8_t cmd[] = { 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }; //0x33 - READ_ROM
	const uint8_t respLen = 8;
	uint8_t response[respLen] = { 0 };

	ibutton.skip();
	const bool status = ibutton.reset();
	if (!status)
	{
		return false;		
	}
	ibutton.send(cmd, sizeof(cmd), response, respLen, sizeof(cmd) - respLen);
	const uint8_t crc = ibutton.crc(response, 7);
	if (response[7] == crc && (crc != 0x00))
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			sn[i] = response[6 - i];
		}  
		return true;
	}
	return false;
}

IBUTTON_KEY_TYPE iButton_Test()
{
	uint8_t cmd[9] = { 0x00 };
	uint8_t response[9] = { 0x00 };

	/* Test for ÒÌ2004 */
	bool status = ibutton.reset();
	if (!status)
	{
		return UNKNOWN;		
	}
	ibutton.skip();
	// 0xAA - read status register
	cmd[0] = 0xAA;
	cmd[1] = 0x00;
	cmd[2] = 0x00;
	cmd[3] = 0xFF;
	ibutton.send(cmd, 4, response, 4, 3);
	if (response[0] == 0x9C)
	{
		//TM2004
		return TM2004;
	}

	/* Test for RW1990.1 */
	status = ibutton.reset();
	if (!status)
	{
		return UNKNOWN;		
	}
	cmd[0] = 0xD1;
	ibutton.send(cmd, 1, nullptr, 0, 0);
	ibutton.sendSlot(1);
	DelayManager::DelayMs(10);
	status = ibutton.reset();
	if (!status)
	{
		return UNKNOWN;		
	}
	cmd[0] = 0xB5;
	cmd[1] = 0xFF;
	ibutton.send(cmd, 2, response, 2, 1);
	if (response[0] != 0xFF)
	{
		//TODO: add additional checks
		return RW_1990_1;	
	}

	/* Test for RW1990.2 */
	status = ibutton.reset();
	if (!status)
	{
		return UNKNOWN;		
	}
	cmd[0] = 0x1D;
	ibutton.send(cmd, 1, nullptr, 0, 0);
	ibutton.sendSlot(0);
	DelayManager::DelayMs(10);
	status = ibutton.reset();
	if (!status)
	{
		return UNKNOWN;		
	}
	cmd[0] = 0x1E;
	cmd[1] = 0xFF;
	ibutton.send(cmd, 2, response, 2, 1);
	if (response[0] == 0xFF)
	{
		status = ibutton.reset();
		if (!status)
		{
			return UNKNOWN;		
		}
		cmd[0] = 0x1D;
		ibutton.send(cmd, 1, nullptr, 0, 0);
		ibutton.sendSlot(1);
		DelayManager::DelayMs(10);
		status = ibutton.reset();
		if (!status)
		{
			return UNKNOWN;		
		}
		cmd[0] = 0x1E;
		cmd[1] = 0xFF;
		ibutton.send(cmd, 2, response, 2, 1);
		if (response[0] != 0xFF)
		{
			return RW_1990_2;
		}
	}

	return UNKNOWN;
}


bool iButton_WriteSN(uint8_t* sn)
{
	uint8_t fullCode[8];	
	fullCode[0] = 0x01; //family code
	for (uint8_t i = 0; i < 6; i++) //serial number
	{
		fullCode[i + 1] = sn[5-i];
	}
	const uint8_t crc = ibutton.crc(fullCode, 7); //crc
	fullCode[7] = crc;

	ibutton.skip();
	bool status = ibutton.reset();
	if (!status)
	{
		return false;		
	}

	uint8_t cmd[] = { 0xD1 }; //0xD1 - allow write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	ibutton.sendSlot(0); //send 0-slot
	DelayManager::DelayMs(10);

	ibutton.skip();
	status = ibutton.reset();
	if (!status)
	{
		return false;		
	}

	//write command 0xD5 and all bytes of serial number
	cmd[0] = 0xD5;  //0xD5 - write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	for (uint8_t i = 0; i < 8; i++) //serial number
	{
		uint8_t byte = fullCode[i];
		ibutton.send(&byte, sizeof(1), nullptr, 0, 0);
		//DelayManager::DelayUs(5);
	}

	status = ibutton.reset();
	if (!status)
	{
		return false;		
	}

	cmd[0] = 0xD1; //0xD1 - block write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	ibutton.sendSlot(1); //send 1-slot
	DelayManager::DelayMs(10);

	return true;
}

void readKeys()
{
	bool keyState = GPIO_ReadInputDataBit(KEYS_PORT, USER_KEY_PIN);
	DelayManager::DelayMs(10);
	keyState = keyState && GPIO_ReadInputDataBit(KEYS_PORT, USER_KEY_PIN);
	if (keyState)
	{
		mode = MODE_WRITE;		
	}
}


void clearSn()
{
}

int main()
{
	bool status;

	uint8_t newSn[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };
	
	Debug_Configuration();
	Clock_Config();
	GPIO_Config();
	USART_Config();
	NVIC_Config();

	DelayManager::SetClock();
	ibutton.init(ONEWIRE_USART);
	
	for (;;)
	{
		readKeys();
		if (mode == MODE_READ)
		{
			clearSn();
			status = iButton_ReadSN(ibuttonSn);
			if (status)
			{
				GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_SET);
				GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_RESET);
			}
			else
			{
				GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
				GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_SET);
			}
		}
		else if (mode == MODE_WRITE)
		{
			GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
			GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_SET);
			status = iButton_WriteSN(newSn);
			//status = iButton_Test();
			if (status)
			{
				GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_RESET);
				for (uint8_t i = 0; i < 5; i++)
				{
					GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_SET);
					DelayManager::DelayMs(200);
					GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
					DelayManager::DelayMs(200);
				}
				clearSn();
				status = iButton_ReadSN(ibuttonSn);
				if (status)
				{
					GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_SET);	
				}
			}
			DelayManager::DelayMs(5000);
			mode = MODE_READ;
		}
		DelayManager::DelayMs(1000);
	}
}
