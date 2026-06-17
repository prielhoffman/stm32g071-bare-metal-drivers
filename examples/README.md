# Examples

This folder contains small embedded C examples used to test and validate the STM32G071 bare-metal drivers.

Each example focuses on one peripheral or communication concept, starting from basic GPIO control and moving toward SPI, I2C, USART, and a small RTC + LCD application.

## Example List

| Folder                           | Description                                                    |
| -------------------------------- | -------------------------------------------------------------- |
| `001_gpio_led_toggle`            | Toggle an LED using GPIO output                                |
| `002_gpio_button_polling`        | Read a push button using GPIO polling                          |
| `003_gpio_button_interrupt`      | Handle a button press using EXTI interrupt                     |
| `004_spi_tx_test`                | Basic SPI transmit test                                        |
| `005_spi_g071_master_f411_slave` | SPI communication between STM32G071 master and STM32F411 slave |
| `006_i2c_master_tx_rx`           | I2C master transmit and receive examples                       |
| `007_usart_tx_rx`                | USART transmit and receive examples                            |
| `008_i2c_rtc_lcd`                | DS1307 RTC and 16x2 I2C LCD project                            |

## Notes

These examples are meant for learning and driver validation.
Some examples use a logic analyzer to verify the actual bus behavior.
