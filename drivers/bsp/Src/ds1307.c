#include "ds1307.h"
#include <string.h>

static void ds1307_i2c_pin_config(void);
static void ds1307_i2c_config(void);
static void ds1307_write(uint8_t value, uint8_t reg_address);
static uint8_t ds1307_read(uint8_t reg_address);

static uint8_t binary_to_bcd(uint8_t value);
static uint8_t bcd_to_binary(uint8_t value);

static I2C_Handle_t ds1307I2CHandle;

/*
 * Initialize DS1307.
 *
 * Later this function will:
 * 1. Configure I2C GPIO pins
 * 2. Configure I2C peripheral
 * 3. Enable I2C peripheral
 * 4. Clear CH bit in seconds register
 * 5. Read back seconds register and return CH bit
 */
uint8_t ds1307_init(void){
    uint8_t seconds = 0U;

    ds1307_i2c_pin_config();
    ds1307_i2c_config();

    /*
     * Read seconds register, clear CH bit and write it back.
     * CH = 1: oscillator stopped
     * CH = 0: oscillator enabled
     */
    seconds = ds1307_read(DS1307_ADDR_SEC);
    seconds &= ~DS1307_SEC_CH_MASK;
    ds1307_write(seconds, DS1307_ADDR_SEC);

    /*
     * Read back the seconds register and return the CH bit state.
     * Return 0 means oscillator is running.
     * Return 1 means CH is still set.
     */
    seconds = ds1307_read(DS1307_ADDR_SEC);

    return (uint8_t)((seconds & DS1307_SEC_CH_MASK) >> DS1307_SEC_CH);
}

/*
 * Set current time into DS1307 timekeeper registers.
 */
void ds1307_set_current_time(RTC_time_t *rtc_time)
{
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;

    seconds = binary_to_bcd(rtc_time->seconds);
    seconds &= ~DS1307_SEC_CH_MASK;
    ds1307_write(seconds, DS1307_ADDR_SEC);

    minutes = binary_to_bcd(rtc_time->minutes);
    ds1307_write(minutes, DS1307_ADDR_MIN);

    hours = binary_to_bcd(rtc_time->hours);
    if (rtc_time->time_format == DS1307_TIME_FORMAT_24HRS){
    	hours &= ~DS1307_HRS_12_24_MASK;
    }
    else{
    	hours |= (uint8_t)(rtc_time->time_format << DS1307_HRS_FORMAT_POS);
    }
    ds1307_write(hours, DS1307_ADDR_HRS);
}

/*
 * Read current time from DS1307 timekeeper registers.
 */
void ds1307_get_current_time(RTC_time_t *rtc_time)
{
    uint8_t seconds = 0;
    uint8_t minutes = 0;
    uint8_t hours = 0;

    seconds = ds1307_read(DS1307_ADDR_SEC);
    seconds &= ~DS1307_SEC_CH_MASK;
    rtc_time->seconds = bcd_to_binary(seconds);

    minutes = ds1307_read(DS1307_ADDR_MIN);
    rtc_time->minutes = bcd_to_binary(minutes);

    hours = ds1307_read(DS1307_ADDR_HRS);
    if (hours & DS1307_HRS_12_24_MASK){
        /*
         * 12-hour format.
         * Keep AM/PM information in time_format and mask format bits before BCD conversion.
         */
    	rtc_time->time_format = (uint8_t)((hours & DS1307_HRS_FORMAT_MASK) >> DS1307_HRS_FORMAT_POS);
    	hours &= ~DS1307_HRS_FORMAT_MASK;
    }
    else{
    	rtc_time->time_format = DS1307_TIME_FORMAT_24HRS;
    }
    rtc_time->hours = bcd_to_binary(hours);
}

/*
 * Set current date into DS1307 timekeeper registers.
 */
void ds1307_set_current_date(RTC_date_t *rtc_date)
{
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;

    day = rtc_date->day;
    ds1307_write(day, DS1307_ADDR_DAY);

    date = binary_to_bcd(rtc_date->date);
    ds1307_write(date, DS1307_ADDR_DATE);

    month = binary_to_bcd(rtc_date->month);
    ds1307_write(month, DS1307_ADDR_MONTH);

    year = binary_to_bcd(rtc_date->year);
    ds1307_write(year, DS1307_ADDR_YEAR);
}

/* Read current date from DS1307 timekeeper registers */
void ds1307_get_current_date(RTC_date_t *rtc_date)
{
    uint8_t date;
    uint8_t month;
    uint8_t year;

    rtc_date->day = ds1307_read(DS1307_ADDR_DAY);

    date = ds1307_read(DS1307_ADDR_DATE);
    rtc_date->date = bcd_to_binary(date);

    month = ds1307_read(DS1307_ADDR_MONTH);
	rtc_date->month = bcd_to_binary(month);

	year = ds1307_read(DS1307_ADDR_YEAR);
	rtc_date->year = bcd_to_binary(year);
}

static void ds1307_i2c_pin_config(void){
	GPIO_Handle_t I2CPins;
	memset(&I2CPins, 0, sizeof(I2CPins));

	I2CPins.pGPIOx = DS1307_I2C_GPIO_PORT;

	I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = DS1307_I2C_PIN_AF;
	I2CPins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
	I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = DS1307_I2C_PUPD;
	I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_HIGH;

	I2CPins.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SCL_PIN;
	GPIO_Init(&I2CPins);

	I2CPins.GPIO_PinConfig.GPIO_PinNumber = DS1307_I2C_SDA_PIN;
	GPIO_Init(&I2CPins);
}

static void ds1307_i2c_config(void){
	ds1307I2CHandle.pI2Cx = DS1307_I2C;

	ds1307I2CHandle.I2CConfig.I2C_SCLSpeed = DS1307_I2C_SPEED;
	ds1307I2CHandle.I2CConfig.I2C_DeviceAddress = 0x61U;
	ds1307I2CHandle.I2CConfig.I2C_ACKControl = I2C_ACK_ENABLE;
	ds1307I2CHandle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;

	I2C_Init(&ds1307I2CHandle);
}

static void ds1307_write(uint8_t value, uint8_t reg_address){
	uint8_t tx[2];

    /*
     * DS1307 write format:
     * first byte = register address, second byte = data.
     */
	tx[0] = reg_address;
	tx[1] = value;

	I2C_MasterSendData(&ds1307I2CHandle, tx, 2U, DS1307_I2C_ADDR);
}

static uint8_t ds1307_read(uint8_t reg_address){
	uint8_t data = 0U;

    /*
     * Set register pointer first, then read one byte from that register.
     */
	I2C_MasterSendData(&ds1307I2CHandle, &reg_address, 1U, DS1307_I2C_ADDR);
	I2C_MasterReceiveData(&ds1307I2CHandle, &data, 1U, DS1307_I2C_ADDR);

	return data;
}

static uint8_t binary_to_bcd(uint8_t value){
    return (uint8_t)(((value / 10U) << 4U) | (value % 10U));
}

static uint8_t bcd_to_binary(uint8_t value){
    return (uint8_t)(((value >> 4U) * 10U) + (value & 0x0FU));
}
