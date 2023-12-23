This project is a weather assistant based on Zephyr RTOS which gets weather information from an external API (probably [open meteo](https://open-meteo.com/)), gets local ambient conditions (temperature and humidity among others) and displays the information in a display.

The aim of the project is to show the versatility of Zephyr RTOS and how it can run the same code (the HTTP call to the weather API) in different boards (starting with STM32), different temperature sensors (starting with bme280) and different displays (such as the ssd1306 OLED display).

# Zephyr version
- **Zephyr version**: v3.5
- **Zephyr SDK**: 0.16.2

For more information such as documentation, please visit the [Zephyr project](https://www.zephyrproject.org/) website.

# TODO list
- [] based code for STM32L4S5 board
- [] Read temperature and humidty from bme280 sensor 
- [] Display local ambient conditions in ssd1306 display
- [] Internet connection for STM32L4S5
- [] Get weather condition from HTTP API