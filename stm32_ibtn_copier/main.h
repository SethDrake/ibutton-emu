#ifndef MAIN_H_
#define MAIN_H_

#define ONEWIRE_PORT  GPIOA
#define ONEWIRE_PIN	  GPIO_Pin_2

#define LED_PORT      GPIOC
#define BLUE_LED_PIN  GPIO_Pin_8
#define GREEN_LED_PIN GPIO_Pin_9

#define KEYS_PORT     GPIOA
#define USER_KEY_PIN  GPIO_Pin_0

#define ONEWIRE_USART USART2

typedef enum
{ 
	MODE_READ = 0,
	MODE_WRITE
} MODE;

typedef enum
{ 
	TM2004	= 0,
	RW_1990_1,
	RW_1990_2,
	UNKNOWN
} IBUTTON_KEY_TYPE;

extern volatile MODE mode;

#endif /* MAIN_H_ */