# Zephyr Weather Assistant 🌦️
Welcome to the Zephyr Weather Assistant Project! 🚀

This innovative project harnesses the power of Zephyr RTOS to create a dynamic Weather Assistant. 🌐 Get real-time weather updates from an external API, including reliable data from [Open Meteo]](https://open-meteo.com/). Simultaneously, gather local ambient conditions displaying this comprehensive information on a user-friendly display.

## Project Objectives 🎯
### 1. Showcase Zephyr's Versatility:
Witness the incredible versatility of Zephyr RTOS as it seamlessly executes the same codebase across diverse hardware environments. Starting with STM32 and expanding further, explore how Zephyr allows the same Weather Assistant code to run on various boards and peripherals, demonstrating adaptability and scalability.

### 2. power of Zephyr as RTOS
The proyect will cover from a the basics of a main loop to advanced topics like thread management, memory sharing, sensor drivers, and display libraries (LVGL), we'll cover it all. Future plans include integrating local weather forecasts and even exploring voice control with TensorFlowLite.


## 🚀 Setting Up Your Zephyr RTOS Environment

### 1. Install Dependencies:

Ensure you have the following dependencies installed on your system:

- [Git](https://git-scm.com/)
- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [python](https://www.python.org/)
- [devicetree compiler](https://www.devicetree.org/)
- Toolchain appropriate for your target architecture (see Install Zephyr SDK section)

Follow the [Zephyr documentation](https://docs.zephyrproject.org/latest/getting_started) for detailed instructions based on your operating system.

### 2. Clone the Zephyr Repository:

Open your terminal and run the following command:

```bash
git clone https://github.com/zephyrproject-rtos/zephyr.git
```

check out the branch into **version 3.5** to get a stable Zephyr version instead of developing directly into main:
```bash
git checkout v3.5-branch
```

### 3. Install Zephyr SDK:

The Zephyr SDK can be downloaded from the [official Zephyr repository](https://github.com/zephyrproject-rtos/sdk-ng/tags). Download the **v0.16.4** version which is the suitable SDK for Zephyr 3.5 release.

### 4. Set Up Environment Variables:

Add the Zephyr binary directory to your PATH and set the `ZEPHYR_BASE` variable. For example:

```bash
export ZEPHYR_SDK_INSTALL_DIR=<path_to_sdk>
source <path_to_zephyr>/zephyr-env.sh
```

### 5. Choose a Board:

Navigate to your project directory and run:

```bash
west init -m https://github.com/zephyrproject-rtos/zephyr
west update
```

## 🎯 Configuration
In order to use the WIFI interface, you need to set your WIFI setting in the `src/wifi.c` file. The SSID and password must be specified in the following code:

```c
// Wifi parameters
wifi_conn_params.ssid = "<your_ssid>";
wifi_conn_params.ssid_length = strlen("<your_ssid>");
wifi_conn_params.psk  = "<your_password>";
wifi_conn_params.psk_length  = strlen("<your_password>");
wifi_conn_params.channel = WIFI_CHANNEL_ANY;
wifi_conn_params.security = WIFI_SECURITY_TYPE_PSK ;
```

## 🛠️ Build the project

Choose your board by running:

```bash
west build -b <your_board>
```

The first version of the weather assistant is developed for STM32L4S5 board. Then, run the following command:
```
west build -b b_l4s5i_iot01a
```

## ⚡️ Flash

Flash it to your board with:

```bash
west flash
```

## 📅 TODO list
- [x] based code for STM32L4S5 board (basic Zephyr structure)
- [x] Read temperature and humidty from bme280 sensor 
- [x] Display local ambient conditions in ssd1306 display
- [x] Internet connection for STM32L4S5
- [x] Get weather condition from HTTP API: open-meteo
- [x] Added a button
- [ ] display manager to change views pressing a button
- [ ] Display weather condition and local ambient conditions in the display