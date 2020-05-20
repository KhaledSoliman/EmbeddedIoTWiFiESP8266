# Embedded Systems Project

## Description

This project was implemented for the course CSCE4301 - Section 1 Embedded Systems

### Author

Khaled Soliman CE Undergraduate

### Project Prompt

Create a small IoT application that can enable the user to perform I/O operations with the STM32 module through a web interface. 

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

## Building

There are two components to this project the arduino code to be flashed onto the ESP module and the keil code to be flashed onto the STM module.

Once you have installed the prequisite software proceed by compiling and flashing the code onto their respective modules.

## Running

To run the code just:
- Power up the modules
- Connect to the WiFi called ESPap using the password which is defaulted on "12345678"
- Enter into your browser window the IP address 192.168.4.1 to access the WiFi module's web server

## Architecture

![alt text](https://github.com/KhaledSoliman/EmbeddedIoTWiFiESP8266/blob/master/Docs/architecture.png "Architecture IMG")

## Connections

![alt text](https://github.com/KhaledSoliman/EmbeddedIoTWiFiESP8266/blob/master/Docs/Connections.png "Connections IMG")

## Functionality

* Retrieving the date/time from RTC module connected to the STM32 module
* Control the STM32 module LEDs status
* Pinging the STM32 module
* Control the WiFi module LEDs status
* Retrieving the temperature from RTC module
* Setting the time on the RTC module
* Setting an alarm on the RTC module

## Communication Structure 
### Structure
`$ID Parameter1 Parameter2...ParameterN\n`

Every command starts with $ and ends with \n, the $ is to distinguish the command ID from command parameters and the \n is to mark the end of the command distinguishing commands from each other
### Commands
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

## Operation Demo

![alt text](https://github.com/KhaledSoliman/EmbeddedIoTWiFiESP8266/blob/master/Docs/OperationSS1.png "Connections IMG")

![alt text](https://github.com/KhaledSoliman/EmbeddedIoTWiFiESP8266/blob/master/Docs/OperationSS2.png "Connections IMG")

## Limitations

* SPIFFS is deprecated and should migrate to LittleFS for better directory structure
* Web Server serves clients on a public socket (Improve websocket communications)
* If any of the HTML or CSS3 files get too large web server faces issues in serving them (Stream data by chunks)
* Implement FreeRTOS or similar functionalities in the microcontroller for better response time/extensibility
* Sync Functionality
