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

void Debug_Configuration();
void Clock_Config();
void GPIO_Config();
void GPIO_Config();
void USART_Config();
void NVIC_Config();

bool iButton_ReadSN();


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

	GPIO_InitStructure.GPIO_Pin = USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_PORT, &GPIO_InitStructure);
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

/*bool iButton_IsWriteAllowed()
{
	bool status = ibutton.reset();
	if (!status)
	{
		return false;		
	}
	uint8_t cmd[] = { 0x1D }; //0xD1 - allow write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	ibutton.sendSlot(1); //send 1-slot
	DelayManager::DelayMs(10);
	status = ibutton.reset();
	if (!status)
	{
		return false;		
	}
	
	uint8_t cmd2[] = { 0x1E, 0xFF };
	uint8_t resp = 0x00;
	ibutton.send(cmd2, sizeof(cmd2), &resp, 1, 1);
	return (resp == 0xFF);
}*/

bool iButton_WriteSN(uint8_t* sn)
{
	uint8_t fullCode[8];	
	bool status = ibutton.reset();
	if (!status)
	{
		return false;		
	}

	fullCode[0] = 0x01; //familty code
	for (uint8_t i = 0; i < 6; i++) //serial number
	{
		fullCode[i + 1] = sn[5-i];
	}
	const uint8_t crc = ibutton.crc(fullCode, 7); //crc
	fullCode[7] = crc;

	/*for (uint8_t i = 0; i < 8; i++)
	{
		fullCode[i] = ~fullCode[i];
	}*/

	uint8_t cmd[] = { 0xD1 }; //0xD1 - allow write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	ibutton.sendSlot(0); //send 1-slot
	DelayManager::DelayMs(10);

	status = ibutton.reset();
	if (!status)
	{
		return false;		
	}
	cmd[0] = 0xD5; //0xD5 - begin write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);

	ibutton.setSlowMode(true);
	//write all bytes of code
	ibutton.send(fullCode, sizeof(fullCode), nullptr, 0, 0);
	ibutton.setSlowMode(false);

	status = ibutton.reset();
	if (!status)
	{
		return false;		
	}

	cmd[0] = 0xD1; //0xD1 - block write
	ibutton.send(cmd, sizeof(cmd), nullptr, 0, 0);
	ibutton.sendSlot(1); //send 0-slot
	DelayManager::DelayMs(10);

	return true;
}

int main()
{
	bool status;

	uint8_t newSn[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
	
	Debug_Configuration();
	Clock_Config();
	GPIO_Config();
	USART_Config();
	NVIC_Config();

	ibutton.init(ONEWIRE_USART);
	/*status = iButton_ReadSN(ibuttonSn);
	if (status)
	{
		status = iButton_WriteSN(newSn);
	}*/
	
	for (;;)
	{
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
		DelayManager::DelayMs(1000);
	}
}
