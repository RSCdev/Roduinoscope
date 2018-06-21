
#ifndef BOARD_CORE_CONFIG_H_
#define BOARD_CORE_CONFIG_H_

#include <roduino.h>

#ifdef CONFIG_RODUINO
#define CONFIG_BOARD 	"RODUINO"
#endif

void board_setup ();

#endif