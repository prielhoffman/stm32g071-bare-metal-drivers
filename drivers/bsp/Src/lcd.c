#include "lcd.h"
#include <string.h>

/*
 * Common PCF8574 LCD backpack bit mapping:
 *
 * bit0 = RS
 * bit1 = RW
 * bit2 = E
 * bit3 = Backlight
 * bit4 = D4
 * bit5 = D5
 * bit6 = D6
 * bit7 = D7
 */

#define LCD_RS				(1u << 0)
#define LCD_RW				(1u << 1)
#define LCD_ENABLE			(1u << 2)
#define LCD_BACKLIGHT		(1u << 3)

static I2C_Handle_t lcdI2CHandle;

static void lcd_i2c_pin_config(void);
static void lcd_i2c_config(void);
static void lcd_send_to_expander(uint8_t data);
static void lcd_mdelay(uint32_t delay_ms);
static void lcd_enable_pulse(uint8_t data);
static void lcd_write_4_bits(uint8_t nibble, uint8_t control_bits);

void lcd_init(void){
	lcd_i2c_pin_config();
	lcd_i2c_config();

    /*
     * Wait after LCD power-up.
     * The LCD controller needs time before receiving commands.
     */
	lcd_mdelay(50U);

	 /*
	  * HD44780 4-bit initialization sequence.
	  * These are sent as raw 4-bit nibbles before normal commands work.
	  */
	lcd_write_4_bits(0x03U, 0U);
	lcd_mdelay(5U);

	lcd_write_4_bits(0x03U, 0U);
	lcd_mdelay(1U);

	lcd_write_4_bits(0x03U, 0U);
	lcd_mdelay(1U);

    /* Switch to 4-bit mode */
	lcd_write_4_bits(0x02U, 0U);
	lcd_mdelay(1U);

    /* Normal LCD commands after 4-bit mode is selected */
    lcd_send_command(0x28U);  /* 4-bit, 2 lines, 5x8 font */
    lcd_send_command(0x0CU);  /* Display ON, cursor OFF */
    lcd_send_command(0x06U);  /* Entry mode: cursor moves right */
    lcd_send_command(0x01U);  /* Clear display */
    lcd_mdelay(2U);
}

void lcd_send_command(uint8_t cmd){
    /*
     * RS = 0: command
     * RW = 0: write
     */
    lcd_write_4_bits((uint8_t)(cmd >> 4U), 0U);		/* High nibble */
    lcd_write_4_bits((uint8_t)(cmd & 0x0FU), 0U);	/* Low nibble */

    /*
     * Delay instead of reading the LCD busy flag.
     * This is simpler when using an I2C backpack.
     */
    lcd_mdelay(2U);
}

void lcd_send_char(uint8_t data){
    /*
     * RS = 1: data
     * RW = 0: write
     */
    lcd_write_4_bits((uint8_t)(data >> 4U), LCD_RS);	/* High nibble */
    lcd_write_4_bits((uint8_t)(data & 0x0FU), LCD_RS); /* Low nibble */

    /* Small delay after writing character data */
    lcd_mdelay(1U);
}

void lcd_send_string(char *message){
    /* Send the string one character at a time */
	while (*message != '\0'){
		lcd_send_char((uint8_t)(*message));
		message++;
	}
}

void lcd_set_cursor(uint8_t row, uint8_t col){
	uint8_t address = 0U;

    /*
     * LCD 16x2 DDRAM layout:
     * Row 0 starts at 0x00
     * Row 1 starts at 0x40
     */
	if (row == 0U){
		address = col;
	}
	else{
		address = (uint8_t)(col + 0x40U);
	}
	lcd_send_command((uint8_t)(0x80U | address));
}

static void lcd_send_to_expander(uint8_t data){
	uint8_t tx_data = data;

	I2C_MasterSendData(&lcdI2CHandle, &tx_data, 1U, LCD_I2C_ADDR);
}

static void lcd_mdelay(uint32_t delay_ms){
    for (uint32_t i = 0U; i < delay_ms; i++){
        for (volatile uint32_t j = 0U; j < 4000U; j++);
    }
}

static void lcd_enable_pulse(uint8_t data){
	lcd_send_to_expander(data | LCD_ENABLE);
	lcd_mdelay(1U);

	lcd_send_to_expander(data & ~LCD_ENABLE);
	lcd_mdelay(1U);
}

static void lcd_write_4_bits(uint8_t nibble, uint8_t control_bits){
	uint8_t data = 0U;

    /*
     * Put the 4-bit nibble on D4-D7.
     * Backpack mapping: bit4=D4, bit5=D5, bit6=D6, bit7=D7.
     */
	data = (uint8_t)((nibble & 0x0F) << 4U);

    /* Keep backlight on and add RS/RW control bits */
	data |= LCD_BACKLIGHT;
	data |= control_bits;

	lcd_send_to_expander(data);
	lcd_enable_pulse(data);
}

static void lcd_i2c_pin_config(void){
	GPIO_Handle_t I2CPins;
	memset(&I2CPins, 0, sizeof(I2CPins));

	I2CPins.pGPIOx = LCD_I2C_GPIO_PORT;

    I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = LCD_I2C_PIN_AF;
    I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = LCD_I2C_PUPD;
    I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

    I2CPins.GPIO_PinConfig.GPIO_PinNumber = LCD_I2C_SCL_PIN;
    GPIO_Init(&I2CPins);

    I2CPins.GPIO_PinConfig.GPIO_PinNumber = LCD_I2C_SDA_PIN;
    GPIO_Init(&I2CPins);
}

static void lcd_i2c_config(void){
	lcdI2CHandle.pI2Cx = LCD_I2C;

	lcdI2CHandle.I2CConfig.I2C_SCLSpeed = LCD_I2C_SPEED;
	lcdI2CHandle.I2CConfig.I2C_DeviceAddress = 0x62U;
	lcdI2CHandle.I2CConfig.I2C_ACKControl = I2C_ACK_ENABLE;
	lcdI2CHandle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;

	I2C_Init(&lcdI2CHandle);
}
