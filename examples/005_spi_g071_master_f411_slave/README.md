# SPI G071 Master ↔ F411 Slave

This example demonstrates SPI communication between two STM32 Nucleo boards:

* **STM32G071RB** configured as SPI master
* **STM32F411RE** configured as SPI slave

The goal of this example is to validate SPI full-duplex communication, software-controlled NSS, command-based communication, and interrupt-based message reception.

## Hardware Setup

### SPI Connections

| STM32G071 Master | STM32F411 Slave | Signal        |
| ---------------- | --------------- | ------------- |
| PB12             | PB12            | NSS           |
| PB13             | PB13            | SCK           |
| PB14             | PB14            | MISO          |
| PB15             | PB15            | MOSI          |
| GND              | GND             | Common ground |

## Files

### Command Handling

These files implement a command-based SPI protocol between the G071 master and the F411 slave.

* `G071_master_spi_cmd_handling.c` — sends command bytes and handles slave responses.
* `F411_slave_spi_cmd_handling.c` — receives commands and responds with ACK/data.

Supported commands:

| Command               |  Value | Description                    |
| --------------------- | -----: | ------------------------------ |
| `COMMAND_LED_CTRL`    | `0x50` | Control LED on the slave board |
| `COMMAND_SENSOR_READ` | `0x51` | Read mock sensor value         |
| `COMMAND_LED_READ`    | `0x52` | Read slave LED status          |
| `COMMAND_PRINT`       | `0x53` | Send text message to slave     |
| `COMMAND_ID_READ`     | `0x54` | Read slave board ID string     |

### Interrupt-Based Message Transfer

These files demonstrate receiving a message from the slave using SPI interrupts.

* `G071_master_spi_msg_rcv_it.c` — receives bytes using SPI interrupt mode.
* `F411_slave_spi_msg_send_it.c` — sends a string message to the master.

A separate Data Ready line is used so the slave can notify the master before the SPI transfer starts.

## Notes

SPI is full-duplex, so dummy bytes are used when one side needs to generate clock pulses while receiving data.

This example is part of the bare-metal SPI driver validation flow.
