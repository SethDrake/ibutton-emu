#include "stm8s.h"
#include "onewire.h"



static void ConfigClock(void);
static void ConfigGPIO(void);
static void ConfigEXTI(void);
static void delay(uint32_t t);
    
static void ConfigClock(void) {
			CLK_DeInit();
                
			CLK_HSECmd(DISABLE);
      CLK_LSICmd(DISABLE);
      CLK_HSICmd(ENABLE);
      while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == FALSE);
			CLK_ClockSwitchCmd(ENABLE);
      CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
      CLK_SYSCLKConfig(CLK_PRESCALER_CPUDIV2);
			CLK_ClockSwitchConfig(CLK_SWITCHMODE_AUTO, CLK_SOURCE_HSI, DISABLE, CLK_CURRENTCLOCKSTATE_ENABLE);
}

static void ConfigGPIO(void) {
		GPIO_DeInit(LED_PORT);
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_HIGH_FAST);
		
		GPIO_DeInit(OW_PORT);
    GPIO_Init(OW_PORT, OW_PIN_RX, GPIO_MODE_IN_FL_IT);
    //GPIO_Init(OW_PORT, OW_PIN_TX, GPIO_MODE_OUT_OD_LOW_FAST);
}		
		
static void ConfigEXTI(void) {
	EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOC, EXTI_SENSITIVITY_FALL_ONLY);
  EXTI_SetTLISensitivity(EXTI_TLISENSITIVITY_FALL_ONLY);
  
  enableInterrupts();
}

static void delay(uint32_t t) {
    while(--t);
}

void main(void)
{
		uint32_t t;
		
		ConfigClock();
		ConfigGPIO();
		GPIO_WriteHigh(OW_PORT, OW_PIN_TX);
		GPIO_WriteLow(LED_PORT, LED_PIN);
		delay(30000);
		GPIO_WriteHigh(LED_PORT, LED_PIN);
		ConfigEXTI();
		
		
		current_state = IDLE;
		
		while (1)
		{
				t++;
				//delay(10);
        //GPIO_WriteReverse(LED_PORT, LED_PIN);
		}
}