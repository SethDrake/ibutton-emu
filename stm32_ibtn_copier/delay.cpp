
#include "delay.h"
#include "stm32f10x_rcc.h"

__IO uint32_t DelayManager::timingDelay;
__IO uint64_t DelayManager::sysTickCount;
__IO uint32_t DelayManager::hclkFreq;

DelayManager::DelayManager() {
	this->sysTickCount = 0;
	this->timingDelay = 0;
	this->hclkFreq = 0;
}

DelayManager::~DelayManager() {

}


void DelayManager::SetClock()
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	hclkFreq = RCC_Clocks.HCLK_Frequency;
}

void DelayManager::Delay(const volatile uint32_t nTimeMs){
  timingDelay = nTimeMs;
	while (timingDelay > 0)
	{
#ifdef OW_GIVE_TICK_RTOS
		vTaskDelay(1);
#endif
	}
}

void DelayManager::TimingDelay_Decrement(){
  if (timingDelay > 0){
    timingDelay--;
  }
}

void DelayManager::SysTickIncrement() {
	sysTickCount++;
	TimingDelay_Decrement();
}

uint64_t DelayManager::GetSysTickCount() {
	return sysTickCount;
}

void DelayManager::DelayMs(const volatile uint32_t nTime)
{
#ifdef OW_GIVE_TICK_RTOS
	vTaskDelay(nTime);
#else
	volatile uint32_t nCount = nTime;
	while (nCount > 0)
	{
		DelayUs(800);
		nCount--;
	}
#endif
}

#define delayUS_ASM(us) do {\
	asm volatile (	"MOV R0,%[loops]\n\t"\
			"1: \n\t"\
			"SUB R0, #1\n\t"\
			"CMP R0, #0\n\t"\
			"BNE 1b \n\t" : : [loops] "r" (10*us) : "memory"\
		      );\
} while(0)

void DelayManager::DelayUs(const volatile uint32_t nTime)
{
	delayUS_ASM(nTime);
}


