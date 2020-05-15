# EmbeddedIoTWiFiESP8266

## Hardware Components

* Nucleo STM32L432KC/Blue Pill
* Wemos D1 Mini ESP8266MOD
* RTC DS3231
* MicroUSB Cable
* 5V Buzzer

## Prequisite Software

* Arduino v1.8.12
* ES8266 Library
* SPIFFS Plugin
* STM32CubeMX
* Keil uVision 5

## Architecture

![alt text](https://github.com/ "Architecture IMG")

## Connections

![alt text](https://github.com/ "Connections IMG")

## Functionality

* Retrieving the date/time from RTC module connected to the STM32 module
* Control the STM32 module LEDs status
* Pinging the STM32 module
* Control the WiFi module LEDs status
* Retrieving the temperature from RTC module
* Setting the time on the RTC module
* Setting an alarm on the RTC module

## Command Structure 

| Command | Description |
|---|---|
| $P STM | Ping STM module |
| $LB 1 | Turn blue LED of ESP module ON |
| $LB 0 | Turn blue LED of ESP module OFF |
| $LB | Toggle blue LED of ESP module |
| $LG 1 | Turn green LED of STM module ON |
| $LG 0 | Turn green LED of STM module OFF |
| $LG | Toggle green LED of STM module |
| $S Time XX:XX:XX | Set time of RTC module |
| $G Time | Get time of RTC module |
| $S Alarm XX:XX:XX | Set alarm of RTC module |
| $G Alarm | Get alarm status of RTC module |
| $G Temp | Get temperature of RTC module |

## Limitations

* SPIFFS is deprecated and should migrate to LittleFS for better directory structure
* Web Server serves clients on a public socket (Improve websocket communications)
* If any of the HTML or CSS3 files get too large web server faces issues in serving them (Stream data by chunks)
* Implement FreeRTOS or similar functionalities in the microcontroller for better response time/extensibility
