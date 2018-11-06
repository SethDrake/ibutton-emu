#include "onewire.h"

SYSTEM_STATES current_state;
uint8_t current_cmd = 0x00;
uint8_t current_bit = 0x00;

static uint8_t getLineState(void) {
		return (GPIO_ReadInputData(OW_PORT) & OW_PIN_RX);
}

static void setLineState(uint8_t v) {
		if (v == 0x00) {
				disableInterrupts();
				//GPIO_WriteLow(OW_PORT, OW_PIN_TX);
				enableInterrupts();
		} else {
				//GPIO_WriteHigh(OW_PORT, OW_PIN_TX);
		}
}

void switchCommand(void) {
		if (current_cmd == 0x33) {
				current_state = READ_ROM;
		} 
		else if (current_cmd == 0xCC) {
				current_state = SKIP_ROM;
		}
		else if (current_cmd == 0x55) {
				current_state = MATCH_ROM;
		}
		else if (current_cmd == 0xF0) {
				current_state = SEARCH_ROM;
		}
		else {
				current_state = IDLE;	
		}
}

void processEvent(void) {
		if (current_state == IDLE) { //check for reset pulse
				_delay_us(15);
				if (getLineState() == 0x01) { //if one after 15uS - one
						current_state = READ_COMMAND;
				    current_cmd |= 0x01;
						current_bit++;
						return;		
				}
				_delay_us(105);
				if (getLineState() == 0x01) { //if one after 120uS - zero
						current_state = READ_COMMAND;
						current_bit++;
						return;		
				}
				_delay_us(360);
				if (getLineState() == 0x00) // if zero after 480us - reset detected
				{
				    current_state = PRESENCE;
						current_cmd = 0x00;
						current_bit = 0;
						//GPIO_WriteLow(LED_PORT, LED_PIN);
						_delay_us(1000);
		    }
		}
		if (current_state == PRESENCE) { //presence answer
				setLineState(0);
				_delay_us(120);
				setLineState(1);
				current_state = IDLE;
		}
		if (current_state == READ_COMMAND) { //read command
				_delay_us(15);
				if (getLineState() == 0x01) { //if one after 15uS - one
				    current_cmd |= 0x01;
						current_bit++;
						if (current_bit >= 7) {
								switchCommand();
						}
						return;		
				}
				_delay_us(105);
				if (getLineState() == 0x01) { //if one after 120uS - zero
						current_bit++;
						if (current_bit >= 7) {
								switchCommand();
						}
						return;		
				}
		}
		if (current_state == READ_ROM) {
				//GPIO_WriteLow(LED_PORT, LED_PIN);
				_delay_us(5000);
				current_state = IDLE;
		}
}