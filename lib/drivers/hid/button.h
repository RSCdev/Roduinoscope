
/* Human Interface Device - Button */

#ifndef BUTTON_CONFIG_H_
#define BUTTON_CONFIG_H_

#include <defconfig.h>
#include <board_core.h>
#include <Arduino.h>

struct button_data {
	byte pin;
	void (*on_press)(void);
};

void button_listener (button_data* button) 
{
	attachInterrupt(digitalPinToInterrupt(button->pin), 
		button->on_press, RISING);
}

#endif