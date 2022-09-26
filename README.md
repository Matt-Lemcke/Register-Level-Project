# Register-Level-Project
Firmware developed in C for the STM32L429 microcontroller with custom hardware abstraction for minimal memory usage.

## Purpose
This is a learning project with a focus on bare-metal programming of ARM-based devices. The goal is to develop firmware using the STMirco CMSIS library for register level programming of peripherals such as:
- GPIO
- ADC
- DMA
- I2C
- Timers

All information regarding peripheral configuration and control was taken from the [reference manual](https://www.st.com/resource/en/reference_manual/dm00031020-stm32f405-415-stm32f407-417-stm32f427-437-and-stm32f429-439-advanced-arm-based-32-bit-mcus-stmicroelectronics.pdf).

## Goal
Create driver code for interfaceing with an EEPROM ASIC via I2C communication protocol with the following requirements:
- Proper abstraction and modularity for portable code and an easy to use interface
- No use of the hardware abstraction library providied by STMicro
- Clean, readable code with comments when neccessary
- Optimized memory management techniques for reading/writing to the 8K EEPROM

## Hardware
- Development board: [STM32F429 Discovery](https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf)
- EEPROM chip: [24PC64F](https://github.com/Matt-Lemcke/Register-Level-Project/blob/main/24FC64F.pdf)

## What Does It Do?
This project is an 8-bit data logger that stores temperature measurements taken at 30 second intervals. The temperature data is stored on an 8K byte external EEPROM memory chip to prevent log data loss during a system reset/power off. The application layer of this device was designed for reading indoor ambient temperatures at regular intervals, but could be changed for more specific use cases using the same custom device drivers.

## Notable Achievemets
- Extends the life of the EEPROM chip by 64 times by using a paging technique to reduce the number of writes to the current data pointer
- Reduced the flash memory footprint of the source code by xx% by using a custom hardware abstraction layer
