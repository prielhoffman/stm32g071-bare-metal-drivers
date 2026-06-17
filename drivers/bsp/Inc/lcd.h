#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>
#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_i2c_driver.h"

/*
 * I2C LCD backpack address.
 * This project uses a common PCF8574-based 16x2 LCD module.
 */
#define  LCD_I2C_ADDR	0x27U

/*
 * LCD uses the same I2C1 bus as the DS1307 RTC.
 * STM32G071 pins:
 * PB8 = I2C1_SCL, PB9 = I2C1_SDA, AF6.
 */
#define LCD_I2C                   I2C1
#define LCD_I2C_GPIO_PORT         GPIOB
#define LCD_I2C_SCL_PIN           GPIO_PIN_NO_8
#define LCD_I2C_SDA_PIN           GPIO_PIN_NO_9
#define LCD_I2C_PIN_AF            6U
#define LCD_I2C_SPEED             I2C_SCL_SPEED_SM
#define LCD_I2C_PUPD              GPIO_PU

/*
 * Public LCD API.
 * Row and column values in lcd_set_cursor() are zero-based:
 * row 0 = first line, row 1 = second line.
 */
void lcd_init(void);
void lcd_send_command(uint8_t cmd);
void lcd_send_char(uint8_t data);
void lcd_send_string(char *message);
void lcd_set_cursor(uint8_t row, uint8_t col);

#endif
