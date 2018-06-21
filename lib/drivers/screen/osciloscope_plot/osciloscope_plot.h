
#ifndef OSCILOSCOPE_PLOT_CONFIG_H_
#define OSCILOSCOPE_PLOT_CONFIG_H_

#include <defconfig.h>
#include <board_core.h>
#include <Arduino.h>
#include <limits.h>

struct osciloscope_plot_point {
	unsigned int x;
	unsigned int y;
};

struct osciloscope_plot_data {
	osciloscope_plot_point * screen_buffer;
	volatile unsigned int iterator = 0;
	volatile unsigned long bias_max = 1023;
	volatile unsigned long bias_min = 0;
	unsigned long read_buffer[CONFIG_OSCILOSCOPE_CHUNK_SIZE];
	volatile float voltage_input = 5;
	volatile float freq = 0;
	volatile float read_init;
	volatile float read_end;
};

void osciloscope_plot_init () 
{
	/* welcome screen */
	TFT.fillScreen(ST7735_BLACK);
	TFT.setTextColor(ST7735_WHITE);
	TFT.setFont();
	TFT.setTextSize(1);
	TFT.setCursor(0, 0);
	TFT.println("MICRO HOBBY OSCILOSCOPE");
	TFT.setCursor(0, 14);
	TFT.print("VERSION :: ");
	TFT.println(CONFIG_OSCILOSCOPE_VERSION);
	TFT.setCursor(0, 28);
	TFT.print("BOARD :: ");
	TFT.println(CONFIG_BOARD);

	delay(2000);
}

void osciloscope_plot_new (osciloscope_plot_data * data) 
{
	data->screen_buffer = 
		new osciloscope_plot_point [CONFIG_OSCILOSCOPE_SCREEN_HEIGHT];
}

void osciloscope_plot_clear (osciloscope_plot_data * data) {
	osciloscope_plot_new(data);

	/* clear screen */
	TFT.fillScreen(ST7735_BLACK);
}

void osciloscope_plot_clear_collumn (osciloscope_plot_point* point) 
{
	unsigned int i = 0;

	for (i = 0; i <= CONFIG_OSCILOSCOPE_SCREEN_HEIGHT; i++) {
		TFT.drawPixel(point->x, 
			(i + CONFIG_OSCILOSCOPE_SCREEN_MIN_OFFSET), 
			ST7735_BLACK);
	}
}

void osciloscope_plot_path (osciloscope_plot_data* data) 
{
	unsigned long diff, i;
	osciloscope_plot_point* point = &data->screen_buffer[data->iterator];

	if ((data->iterator -1) >= 0 && 
		(data->iterator -1) < CONFIG_OSCILOSCOPE_SCREEN_WIDTH) {
		
		osciloscope_plot_point ant = data->
			screen_buffer[data->iterator -1]; 

		if (point->y > ant.y) {

			diff = point->y - ant.y;

			for (i = 0; i < diff; i++) {
				TFT.drawPixel(point->x, (ant.y + i), 
					ST7735_GREEN);
			}
		} else {

			diff = ant.y - point->y;

			for (i = 0; i < diff; i++) {
				TFT.drawPixel(point->x, (ant.y - i), 
					ST7735_GREEN);
			}
		}
	}
}

void osciloscope_plot_draw_point (osciloscope_plot_data* data, 
	unsigned long value) 
{
	unsigned long fit_value;
	unsigned long diff;
	osciloscope_plot_point* point = &data->screen_buffer[data->iterator];

	/* invert WARNING THE SCREEN IS FLIPPED */
	value = abs(1023 - value);
	point->x = data->iterator;
	
	/* fit the value inside my bias */
	if (data->bias_max != 1023 && data->bias_min != 0) {
		diff = data->bias_max - data->bias_min;
		fit_value = ((diff - (data->bias_max - value)) 
			* CONFIG_OSCILOSCOPE_SCREEN_HEIGHT) / diff;
	} else {
		/* fit the value inside my screen */
		fit_value = (value * CONFIG_OSCILOSCOPE_SCREEN_HEIGHT);
		fit_value /= 1023;
	}
	fit_value += CONFIG_OSCILOSCOPE_SCREEN_MIN_OFFSET;
	
	/* sanity */
	if (fit_value > CONFIG_OSCILOSCOPE_SCREEN_MAX_OFFSET ||
		fit_value < CONFIG_OSCILOSCOPE_SCREEN_MIN_OFFSET)
		fit_value = CONFIG_OSCILOSCOPE_SCREEN_MIN_OFFSET;

	point->y = fit_value;

	/* clear and plot */
	osciloscope_plot_clear_collumn(point);
	TFT.drawPixel(point->x, 
		point->y, ST7735_GREEN);

	/* plot path */
	osciloscope_plot_path(data);

#ifdef CONFIG_DEBUG
	/* WARNING */
	/* FOR FAST PLOT TURN OF CONFIG DEBUG */
	Serial.print("PIN_VAL: ");
	Serial.print(value);
	Serial.print(" FIT_VAL: ");
	Serial.println(fit_value);
	Serial.print(" ITERATOR: ");
	Serial.println(data->iterator);
	delay(50);
#endif
}

void osciloscope_plot_read_pin_and_plot (osciloscope_plot_data * data)
{
	unsigned long pin_value;

	/* read the analog pin and fit it in screen height */
	pin_value = analogRead(CONFIG_OSCILOSCOPE_INPUT_PIN);

	/* draw point */
	osciloscope_plot_draw_point(data, pin_value);

	/* ring buffer */
	if (data->iterator < CONFIG_OSCILOSCOPE_SCREEN_WIDTH)
		data->iterator++;
	else
		data->iterator = 0;
}

unsigned long* osciloscope_plot_read_chunk (osciloscope_plot_data* data) 
{
	unsigned int i;

	for (; i < CONFIG_OSCILOSCOPE_CHUNK_SIZE; i++) {
		data->read_buffer[i] = analogRead(CONFIG_OSCILOSCOPE_INPUT_PIN);
	}

	return data->read_buffer;
}

void osciloscope_plot_draw_chunk (osciloscope_plot_data* data) 
{
	unsigned long* chunk;
	unsigned int i;

	/* get the moment chunk */
	chunk = osciloscope_plot_read_chunk(data);

	/* plot all chunk points with path */
	for (i = 0; i < CONFIG_OSCILOSCOPE_CHUNK_SIZE; i++) {
		data->iterator = i;
		osciloscope_plot_draw_point(data, chunk[i]);
	}
}

void osciloscope_plot_calc_freq (osciloscope_plot_data* data) 
{
	data->read_init = micros();
	pulseIn(CONFIG_OSCILOSCOPE_INPUT_PIN, HIGH);	
	pulseIn(CONFIG_OSCILOSCOPE_INPUT_PIN, LOW);
	data->read_end = micros();

	data->freq = data->read_end - data->read_end;
	/* to seconds */
	data->freq = data->freq / 1000000; 
	data->freq = 1 / data->freq;

	TFT.print("Freq: ");
	TFT.print(data->freq);
	TFT.print("Hz");
}

/* calc a bias between the min value and max from a chunk */
void osciloscope_plot_calc_bias (osciloscope_plot_data* data) 
{
	unsigned int i;
	unsigned long 	max = 0,
			min = LONG_MAX;

	/* plot all chunk points with path */
	for (i = 0; i < CONFIG_OSCILOSCOPE_CHUNK_SIZE; i++) {
		if (max < abs(1023 - data->read_buffer[i]))
			max = abs(1023 - data->read_buffer[i]);

		if (min > abs(1023 - data->read_buffer[i]))
			min = abs(1023 - data->read_buffer[i]);
	}

	/* fit inside offset */
	data->bias_max = max;
	data->bias_min = min;
	data->voltage_input = max * (5.0 / 1023.0);
	TFT.setCursor(1, 2);
	TFT.print("Volt: ");
	TFT.println(data->voltage_input);

	/* check frequency */
	/*osciloscope_plot_calc_freq(data);*/
}

void osciloscope_plot_recalc_bias (osciloscope_plot_data* data) 
{
	TFT.fillScreen(ST7735_BLACK);
	osciloscope_plot_read_chunk(data);
	osciloscope_plot_calc_bias(data);
}

#endif