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

int main(void)
{
	int rc = 0;

	/* Init the sensor variables */
	memset(&temperature_SV, 0, sizeof(struct sensor_value));
	memset(&humidity_SV, 0, sizeof(struct sensor_value));
	
	if (!device_is_ready(temp_humd_device)) {
		LOG_ERR("Device %s is not ready", temp_humd_device->name);
	} else {
		LOG_INF("Device %s is ready", temp_humd_device->name);
	}

	while(1){
		/* Sampling a new sensor value */
		rc = sensor_sample_fetch(temp_humd_device);
		if(rc != 0){
			LOG_ERR("error fetching the sensor. Error: %d", rc);
			break;
		}

		rc = sensor_channel_get(temp_humd_device, SENSOR_CHAN_AMBIENT_TEMP, &temperature_SV);
		if(rc != 0){
			LOG_ERR("sensor get SENSOR_CHAN_AMBIENT_TEMP failed. Error: %d", rc);
			break;
		}

		rc = sensor_channel_get(temp_humd_device, SENSOR_CHAN_HUMIDITY, &humidity_SV);
		if(rc != 0){
			LOG_ERR("sensor get SENSOR_CHAN_HUMIDITY failed. Error: %d", rc);
			break;
		}

		LOG_INF("Temperature: %d.%d. Humidity: %d.%d", 
					temperature_SV.val1, temperature_SV.val2/10000,
					humidity_SV.val1, humidity_SV.val2/10000);

		k_msleep(500);
	}

	LOG_ERR("end of app");

	return 0;
}
