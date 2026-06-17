# USART TX/RX

This folder contains USART examples used to test and validate the STM32G071 bare-metal USART driver.

The examples demonstrate basic USART transmission and bidirectional communication between STM32 boards.

## Files

| File            | Description                        |
| --------------- | ---------------------------------- |
| `usart_tx.c`    | USART transmit-only example        |
| `usart_tx_rx.c` | USART transmit and receive example |

## What This Example Covers

* USART GPIO alternate function configuration
* Baud rate configuration
* Blocking transmit
* Blocking receive
* TX/RX signal validation using a logic analyzer

## Hardware

* Board: NUCLEO-G071RB
* Peripheral: USART
* Logic analyzer connected to TX/RX lines

Some tests use a second STM32 board for bidirectional communication.

## Logic Analyzer Captures

### USART Transmit

The capture below shows the STM32G071 transmitting a string over USART TX.

![USART transmit](logic_analyzer/usart_tx.png)

### USART TX/RX

The capture below shows bidirectional USART communication between two STM32 boards.

![USART TX RX](logic_analyzer/usart_tx_rx.png)

## Notes

USART communication was validated by decoding the TX and RX lines with a logic analyzer.
