/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

#define LED_PIN  GPIO_PIN_5
#define LED_PORT GPIOB

static void ConfigClock(void);
static void ConfigGPIO(void);
static void delay(uint32_t t);
    

void main(void)
{
		ConfigClock();
		ConfigGPIO();
		
		while (1)
		{
				delay(30000);
        GPIO_WriteReverse(LED_PORT, LED_PIN);
		}
}

static void delay(uint32_t t) {
    while(--t);
}

static void ConfigClock(void) {
			CLK_DeInit();
                
			CLK_HSECmd(DISABLE);
      CLK_LSICmd(DISABLE);
      CLK_HSICmd(ENABLE);
      while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == FALSE);
			CLK_ClockSwitchCmd(ENABLE);
      CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV8);
      CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV1);
			CLK_SYSCLKConfig(CLK_PRESCALER_HSIDIV8);
			CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, DISABLE, CLK_CURRENTCLOCKSTATE_ENABLE);
}

static void ConfigGPIO(void) {
		GPIO_DeInit(LED_PORT);
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_FAST);
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif