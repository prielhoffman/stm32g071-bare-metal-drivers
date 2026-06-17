# I2C Master TX/RX

This folder contains I2C examples used to test and validate the STM32G071 bare-metal I2C driver.

The examples cover basic master transmit, master receive, interrupt-based receive, and a simple slave-side string transmit test.

## Files

| File                         | Description                                                     |
| ---------------------------- | --------------------------------------------------------------- |
| `i2c_master_tx_testing.c`    | I2C master transmit example                                     |
| `i2c_master_rx_testing.c`    | I2C master receive example using blocking mode                  |
| `i2c_master_rx_testing_it.c` | I2C master receive example using interrupt mode                 |
| `i2c_slave_tx_string.c`      | Simple slave-side example that transmits a string to the master |

## What This Example Covers

* I2C GPIO alternate function configuration
* I2C master transmit
* I2C master receive
* Interrupt-based I2C receive flow
* ACK/NACK behavior
* Logic analyzer validation of I2C transfers

## Hardware

* Board: NUCLEO-G071RB
* Peripheral: I2C1
* Pins:

  * PB8 → I2C1_SCL
  * PB9 → I2C1_SDA

Some tests use a second board as the I2C slave.

## Logic Analyzer Captures

### I2C Master Transmit

The master writes data to the I2C slave address and receives ACK responses.

![I2C master transmit](logic_analyzer/i2c_master_tx_logic.png)

### I2C Master Receive

The master reads data from the I2C slave.

![I2C master receive](logic_analyzer/i2c_master_rx_logic.png)

### I2C Master Receive Using Interrupts

This capture validates the interrupt-based receive flow.

![I2C master receive interrupt](logic_analyzer/i2c_master_rx_it_logic.png)

### Slave String Transmit

The slave transmits a string to the master, and the decoded bytes can be seen on the I2C bus.

![I2C slave string transmit](logic_analyzer/i2c_slave_tx_string_logic.png)

## Notes

These examples were used to verify the custom I2C driver behavior on the physical bus using a logic analyzer.
