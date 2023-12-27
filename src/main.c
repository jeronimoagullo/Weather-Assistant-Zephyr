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
#include <zephyr/drivers/sensor.h>

#define TEMP_SENSOR DT_ALIAS(tempsensor)
const struct device *temp_humd_device = DEVICE_DT_GET(TEMP_SENSOR);

struct sensor_value temperature_SV; 
struct sensor_value humidity_SV;

struct k_timer sampling_timer;

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

	LOG_INF("Temperature: %d.%d. Humidity: %d.%d", 
				temperature_SV.val1, temperature_SV.val2/10000,
				humidity_SV.val1, humidity_SV.val2/10000);
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

int main(void)
{
	/* Init the sensor variables */
	memset(&temperature_SV, 0, sizeof(struct sensor_value));
	memset(&humidity_SV, 0, sizeof(struct sensor_value));
	
	if (!device_is_ready(temp_humd_device)) {
		LOG_ERR("Device %s is not ready", temp_humd_device->name);
	} else {
		LOG_INF("Device %s is ready", temp_humd_device->name);
	}

	/* Init the sampling timer */
	k_timer_init(&sampling_timer, sampling_function, NULL);

	/* start the sampling with period of 500 ms */
	k_timer_start(&sampling_timer, K_NO_WAIT, K_MSEC(500));

	LOG_ERR("end of app");

	return 0;
}
