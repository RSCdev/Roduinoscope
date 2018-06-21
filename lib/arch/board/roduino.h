
#ifndef RODUINO_CONFIG_H_
#define RODUINO_CONFIG_H_

#include <defconfig.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#ifdef CONFIG_RODUINO

static Adafruit_ST7735 TFT = Adafruit_ST7735(
		CONFIG_TFT_CS,  CONFIG_TFT_DC, CONFIG_TFT_RST);

void board_setup ()
{
#ifdef CONFIG_DEBUG
	/* debug serial */
	Serial.begin(CONFIG_SERIAL_BAUD_RATE);
#endif

	/* init direction for analog input */
	pinMode(CONFIG_OSCILOSCOPE_INPUT_PIN, INPUT_PULLUP);
	/* init direction for interrupt button calc bias */
	pinMode(CONFIG_BUTTON_CALC_INTERRUPT, INPUT);

	/* init screen */
	TFT.initR(INITR_BLACKTAB);
	TFT.setRotation(1);
}


#endif

#endif