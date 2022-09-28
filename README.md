# Register-Level-Project
Firmware developed in C for the STM32F429 microcontroller with custom hardware abstraction for minimal memory usage.

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
- Communication error handling
- Proper abstraction and modularity for portable code and an easy to use interface
- Clean, readable code with comments when neccessary

## Hardware
- Development board: [STM32F429 Discovery](https://www.st.com/resource/en/user_manual/um1670-discovery-kit-with-stm32f429zi-mcu-stmicroelectronics.pdf)
- EEPROM chip: [24PC64F](https://github.com/Matt-Lemcke/Register-Level-Project/blob/main/24FC64F.pdf)
