# STM32G071 Bare-Metal Drivers

This repository contains bare-metal peripheral drivers and example projects for the **STM32G071RB** microcontroller, developed as part of my embedded systems learning process.

The goal of this repository is to practice low-level embedded C development by working directly with peripheral registers and writing reusable drivers for common MCU peripherals.

## Target Hardware

* **Board:** NUCLEO-G071RB
* **MCU:** STM32G071RBTx
* **Core:** Arm Cortex-M0+
* **IDE:** STM32CubeIDE
* **Language:** C

Additional boards and modules used in some examples:

* NUCLEO-F411RE as a secondary SPI/I2C test board
* DS1307 RTC module
* 16x2 I2C LCD module
* Logic analyzer for protocol validation

## Repository Structure

```text
drivers/
├── Inc/        # Device header and peripheral driver headers
├── Src/        # Peripheral driver source files
└── bsp/        # Board support modules, such as DS1307 RTC and I2C LCD

examples/
├── 001_gpio_led_toggle/
├── 002_gpio_button_polling/
├── 003_gpio_button_interrupt/
├── 004_spi_tx_test/
├── 005_spi_g071_master_f411_slave/
├── 006_i2c_master_tx_rx/
├── 007_usart_tx_rx/
└── 008_i2c_rtc_lcd/
```

## Implemented Drivers

The repository includes bare-metal drivers for:

* GPIO
* External interrupts / EXTI
* SPI
* I2C
* USART
* RCC clock helper functions

The drivers are written specifically for the STM32G071 register layout and are not based on HAL.

## Example Projects

The `examples/` folder contains small projects used to test and validate the drivers.

Examples include:

* LED toggle using GPIO
* Button polling and button interrupt handling
* SPI transmit test
* SPI communication between STM32G071 master and STM32F411 slave
* I2C master transmit and receive examples
* USART transmit and receive examples
* RTC + I2C LCD project using DS1307 and a 16x2 LCD

## RTC LCD Project

One of the final examples combines multiple modules into a small working application:

* DS1307 RTC over I2C
* 16x2 LCD over I2C
* Two I2C slaves on the same bus
* SysTick-based periodic display update
* Logic analyzer validation

The LCD displays the current time and date read from the RTC.

## Validation

Several examples were tested using a logic analyzer to verify protocol behavior on the physical bus.

Captured protocols include:

* SPI
* I2C
* USART

These captures are included in the relevant example folders when available.

## Notes

This repository is intended for learning and portfolio purposes.
The code focuses on understanding low-level MCU behavior, peripheral configuration, register access, and communication protocols.

Some examples are intentionally simple and are meant to demonstrate one concept at a time.
