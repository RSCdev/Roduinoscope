#include <Arduino.h>
#include <SPI.h>
#include <defconfig.h>
#include <board_core.h>
#include <osciloscope_plot.h>
#include <button.h>

/* MAIN ENTRY */
osciloscope_plot_data screen_plot;
button_data calc_button;

void setup() 
{
	/* Board specific initialization */
	board_setup();

	/* init button */
	calc_button.pin = CONFIG_BUTTON_CALC_INTERRUPT;
	/* on press recalc the bias */
	calc_button.on_press = [] () {
		osciloscope_plot_recalc_bias(&screen_plot);
	};
	button_listener (&calc_button);

	/* Project */
	osciloscope_plot_init();
	osciloscope_plot_clear(&screen_plot);

	/* get initial chunk */
	osciloscope_plot_draw_chunk(&screen_plot);
	/* calc the bias */
	osciloscope_plot_calc_bias(&screen_plot);
}

void loop() 
{
	/*osciloscope_plot_read_pin_and_plot (&screen_plot);*/
	osciloscope_plot_draw_chunk(&screen_plot);
}