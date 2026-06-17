# SPI TX Test

This example demonstrates a basic SPI transmit test using the STM32G071RB.

The STM32G071 is configured as an SPI master and repeatedly transmits a fixed string over MOSI. This example is used to validate the SPI GPIO configuration, SPI peripheral initialization, and blocking transmit API in the custom bare-metal SPI driver.

## What This Example Covers

* SPI2 GPIO alternate function configuration
* SPI master mode configuration
* Full-duplex SPI setup
* Blocking SPI transmit
* Basic signal validation using SCK and MOSI

## Hardware

* Board: NUCLEO-G071RB
* Peripheral: SPI2
* Pins:

  * PB13 → SPI2_SCK
  * PB15 → SPI2_MOSI

## Notes

This is a transmit-only test. No SPI slave response is required for this example.
The output can be verified with a logic analyzer by probing SCK and MOSI.
