#include <stm32f10x.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "delay.h"
#include "onewire.h"
#include "misc.h"

#define USART_PORT    GPIOA
#define USART_TX_PIN  GPIO_Pin_2
//#define USART_RX_PIN  GPIO_Pin_3

#define MCO_PORT      GPIOA
#define MCO_PIN		  GPIO_Pin_8

#define LED_PORT      GPIOC
#define BLUE_LED_PIN  GPIO_Pin_8
#define GREEN_LED_PIN GPIO_Pin_9

void Clock_Config();
void GPIO_Config();
void GPIO_Config();
void USART_Config();
void NVIC_Config();


void Clock_Config()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB1Periph_USART2 | 
						   RCC_AHBPeriph_DMA1 | RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);
	
	RCC_MCOConfig(RCC_MCO_PLLCLK_Div2);

	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	SysTick_Config(SystemCoreClock / 1000);
}

void GPIO_Config()
{
	GPIO_InitTypeDef GPIO_InitStructure;
 
	GPIO_InitStructure.GPIO_Pin = MCO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(MCO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = BLUE_LED_PIN | GREEN_LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_PORT, &GPIO_InitStructure);

	/*GPIO_InitStructure.GPIO_Pin = USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(USART_PORT, &GPIO_InitStructure);*/
}

void USART_Config() {
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);
	USART_HalfDuplexCmd(USART2, ENABLE);
	USART_Cmd(USART2, ENABLE);
}


void NVIC_Config()
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x05;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

int main()
{
	uint8_t response[8] = {0};
	uint8_t key_sn[6] = {0};
	uint8_t crc = 0x00;
	
	Clock_Config();
	GPIO_Config();
	USART_Config();
	NVIC_Config();
	
	for (;;)
	{
		uint8_t cmd[9] = { 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
		uint8_t status = OW_Send(OW_SEND_RESET, cmd, 9, response, 8, 1);
		if (status == OW_OK)
		{
			crc = OW_CRC(response, 7);
			if (response[7] == crc && (crc != 0x00))
			{
				for (uint8_t i = 0; i < 6; i++)
				{
					key_sn[i] = response[6 - i];
				}  
				GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_SET);
				GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_RESET);
			}
			else
			{
				GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
				GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_SET);
			}
		}
		else
		{
			GPIO_WriteBit(LED_PORT, GREEN_LED_PIN, Bit_RESET);
			GPIO_WriteBit(LED_PORT, BLUE_LED_PIN, Bit_SET);
		}
		DelayManager::DelayMs(1000);
	}
}
