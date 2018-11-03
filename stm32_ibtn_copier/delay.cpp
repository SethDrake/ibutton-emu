
#include "delay.h"
#include "stm32f10x_rcc.h"

__IO uint32_t DelayManager::timingDelay;
__IO uint64_t DelayManager::sysTickCount;

DelayManager::DelayManager() {
	this->sysTickCount = 0;
	this->timingDelay = 0;
}

DelayManager::~DelayManager() {

}

void DelayManager::Delay(const volatile uint32_t nTimeMs){
  timingDelay = nTimeMs;
	while (timingDelay > 0)
	{
#ifdef OW_GIVE_TICK_RTOS
		taskYIELD();
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
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	volatile uint32_t nCount = (RCC_Clocks.HCLK_Frequency / 50000) * nTime;
	while (nCount > 0)
	{
#ifdef OW_GIVE_TICK_RTOS
		taskYIELD();
#endif
		nCount--;
	}
}

void DelayManager::DelayUs(const volatile uint32_t nTime)
{
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);

	volatile uint32_t nCount = (RCC_Clocks.HCLK_Frequency / 50000000) * nTime;
	while (nCount > 0)
	{
		nCount--;
	}
}

