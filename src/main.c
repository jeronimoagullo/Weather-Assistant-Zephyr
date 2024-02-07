/**
 * @file main.c
 * @author Jeronimo Agullo (jeronimoagullo97@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-12-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(weather_assistant, LOG_LEVEL_DBG);

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/display/cfb.h>
#include "cfb_mono_04B.h"
#include "cfb_mono_COMICBD.h"
#include "wifi.h"
#include "open_meteo_http.h"

#define MAX_NUM_FONT_SIZES 3

#define TEMP_SENSOR DT_ALIAS(tempsensor)
const struct device *temp_humd_device = DEVICE_DT_GET(TEMP_SENSOR);

#define CTRL_BUTTON DT_ALIAS(sw0)
const struct device *ctrl_button = DEVICE_DT_GET(CTRL_BUTTON);

static const struct gpio_dt_spec button = 
				GPIO_DT_SPEC_GET_OR(CTRL_BUTTON, gpios,{0});
static struct gpio_callback button_cb_data;

const struct device *display_device;

/* Defining display stack area */
#define DISPLAY_STACK_SIZE 512
#define THREAD_DISPLAY_PRIORITY 5
K_THREAD_STACK_DEFINE(display_stack_area, DISPLAY_STACK_SIZE);
struct k_thread thread_display_data;

struct sensor_value temperature_SV; 
struct sensor_value humidity_SV;

struct k_timer sampling_timer;

/**
 * @brief button callback
 * 
 * @param dev 
 * @param cb 
 * @param pins 
 */
void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	LOG_INF("Button pressed at %" PRIu32 "", k_cycle_get_32());
}

/**
 * @brief work handler for sampling timer. it is needed since 
 * 		  the required work cannot be done at the interrupt level
 * 
 * @param work 
 */
void sampling_work_handler(struct k_work *work){
	int rc = 0;

	/* Sampling a new sensor value */
	rc = sensor_sample_fetch(temp_humd_device);
	if(rc != 0){
		LOG_ERR("error fetching the sensor. Error: %d", rc);
	}

	rc = sensor_channel_get(temp_humd_device, SENSOR_CHAN_AMBIENT_TEMP, &temperature_SV);
	if(rc != 0){
		LOG_ERR("sensor get SENSOR_CHAN_AMBIENT_TEMP failed. Error: %d", rc);
	}

	rc = sensor_channel_get(temp_humd_device, SENSOR_CHAN_HUMIDITY, &humidity_SV);
	if(rc != 0){
		LOG_ERR("sensor get SENSOR_CHAN_HUMIDITY failed. Error: %d", rc);
	}

	LOG_INF("Temperature: %.02fºC. Humidity: %.02f%%", 
				sensor_value_to_double(&temperature_SV),
				sensor_value_to_double(&humidity_SV));
}

K_WORK_DEFINE(sampling_work, sampling_work_handler);

/**
 * @brief expiring function for sampling timer
 * 
 * @param timer_id 
 */
extern void sampling_function(struct k_timer *timer_id){
	k_work_submit(&sampling_work);
}

extern void thread_display(void* v1, void *v2, void *v3){
	uint8_t width, height;
	char buf[20];

	// get display width and height of font with index 0
	cfb_get_font_size(display_device, 0, &width, &height);

	while (1) {
		cfb_framebuffer_clear(display_device, false);
		cfb_framebuffer_set_font(display_device, 0);

		// Display Weather Assistant in lines 1 and 2
		cfb_print(display_device, ">>Ambient:",0,0);

		cfb_framebuffer_set_font(display_device, 1);

		// Display temperature in line 3
		sprintf(buf, "%.02fC", sensor_value_to_double(&temperature_SV));
		cfb_print(display_device, buf, 0, 2 * height);

		// Display humidity in line 4
		sprintf(buf, "%.02f%%", sensor_value_to_double(&humidity_SV));
		cfb_print(display_device, buf, 0, 3 * height);

		// Finalize frame to load it into RAM to be displayed
		cfb_framebuffer_finalize(display_device);
		
		k_msleep(500);
	}
}

int main(void)
{
	uint16_t x_res, y_res;
	uint16_t rows, cols;
	uint8_t width, height;
	uint8_t ppt;
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Error: button device %s is not ready",
		       button.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (gpio_pin_configure_dt(&button, GPIO_INPUT)) {
		LOG_ERR("Error %d: failed to configure %s pin %d",
		       ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
			ret, button.port->name, button.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	

	/* Init display */
	display_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_device)) {
		LOG_ERR("Display %s not found. Aborting sample.",
			display_device->name);
	} else {
		LOG_INF("Display %s is ready", display_device->name);
	}

	/* Init routing for display */
	if (display_set_pixel_format(display_device, PIXEL_FORMAT_MONO10) != 0) {
		if (display_set_pixel_format(display_device, PIXEL_FORMAT_MONO01) != 0) {
			LOG_ERR("Failed to set required pixel format");
		}
	}

	if (cfb_framebuffer_init(display_device)) {
		LOG_ERR("Framebuffer initialization failed!\n");
	}

	cfb_framebuffer_clear(display_device, true);
	display_blanking_off(display_device);

	x_res = cfb_get_display_parameter(display_device, CFB_DISPLAY_WIDTH);
	y_res = cfb_get_display_parameter(display_device, CFB_DISPLAY_HEIGH);
	rows = cfb_get_display_parameter(display_device, CFB_DISPLAY_ROWS);
	cols = cfb_get_display_parameter(display_device, CFB_DISPLAY_COLS);
	ppt = cfb_get_display_parameter(display_device, CFB_DISPLAY_PPT);

	// log display parameters
	LOG_INF("Display parameters: x_res %d, y_res %d, ppt %d, rows %d, cols %d",
	       x_res, y_res, ppt, rows, cols);

	// log supported display fonts
	LOG_INF("number of fonts: %d, indexes and sizes:", cfb_get_numof_fonts(display_device));
	for (int idx = 0; idx < cfb_get_numof_fonts(display_device); idx++) {
		cfb_get_font_size(display_device, idx, &width, &height);
		LOG_INF("font %d -> width %d, height %d", idx, width, height);
	}

	// get display width and height of font with index 0
	cfb_get_font_size(display_device, 0, &width, &height);

	cfb_framebuffer_invert(display_device);

	// Set font kerning (spacing between individual letters).
	cfb_set_kerning(display_device, 0);

	// Display welcome message
	cfb_framebuffer_clear(display_device, false);
	k_msleep(100);

	cfb_framebuffer_set_font(display_device, 0);
	cfb_print(display_device, "Weather", 0, 0);
	cfb_print(display_device, "Assistant!", 0, height);
	cfb_print(display_device, "By", 0, 2 * height);
	cfb_print(display_device, "Jeronimo", (x_res - 8 * width), 3 * height);
	cfb_framebuffer_finalize(display_device);

	k_tid_t idThreadDisplay = k_thread_create(&thread_display_data, display_stack_area,
							K_THREAD_STACK_SIZEOF(display_stack_area),
							thread_display,
							NULL, NULL, NULL,
							THREAD_DISPLAY_PRIORITY, 0, K_MSEC(2000));


	/* Init the sensor variables */
	memset(&temperature_SV, 0, sizeof(struct sensor_value));
	memset(&humidity_SV, 0, sizeof(struct sensor_value));

	/* Init temperature and humidity sensor */
	if (!device_is_ready(temp_humd_device)) {
		LOG_ERR("Sensor %s is not ready", temp_humd_device->name);
	} else {
		LOG_INF("Sensor %s is ready", temp_humd_device->name);
	}

	/* Init the sampling timer */
	k_timer_init(&sampling_timer, sampling_function, NULL);

	/* start the sampling with period of 500 ms */
	k_timer_start(&sampling_timer, K_NO_WAIT, K_MSEC(500));

	init_wifi();

	http_get_open_meteo_forcast();

	LOG_INF("end of main");

	return 0;
}
