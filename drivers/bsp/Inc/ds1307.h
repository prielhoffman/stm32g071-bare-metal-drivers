#ifndef DS1307_H_
#define DS1307_H_

#include <stdint.h>
#include "stm32g071xx.h"
#include "stm32g071xx_gpio_driver.h"
#include "stm32g071xx_i2c_driver.h"

/*
 * DS1307 I2C 7-bit slave address.
 * Datasheet address: 1101000b = 0x68.
 */
#define DS1307_I2C_ADDR              0x68U

/* DS1307 timekeeper register addresses */
#define DS1307_ADDR_SEC              0x00U
#define DS1307_ADDR_MIN              0x01U
#define DS1307_ADDR_HRS              0x02U
#define DS1307_ADDR_DAY              0x03U
#define DS1307_ADDR_DATE             0x04U
#define DS1307_ADDR_MONTH            0x05U
#define DS1307_ADDR_YEAR             0x06U
#define DS1307_ADDR_CONTROL          0x07U

/*
 * DS1307 seconds register special bit.
 * CH = Clock Halt.
 * CH = 1: oscillator disabled.
 * CH = 0: oscillator enabled.
 */
#define DS1307_SEC_CH                7U
#define DS1307_SEC_CH_MASK           (1U << DS1307_SEC_CH)

/*
 * DS1307 hours register special bits.
 */
#define DS1307_HRS_12_24             6U
#define DS1307_HRS_AM_PM             5U

#define DS1307_HRS_12_24_MASK        (1U << DS1307_HRS_12_24)
#define DS1307_HRS_AM_PM_MASK        (1U << DS1307_HRS_AM_PM)

/*
 * Time format options used by the RTC API.
 */
#define DS1307_TIME_FORMAT_12HRS_AM   2U   /* bits[6:5] = 10 */
#define DS1307_TIME_FORMAT_12HRS_PM   3U   /* bits[6:5] = 11 */
#define DS1307_TIME_FORMAT_24HRS      0U   /* bits[6:5] = 00 */

#define DS1307_HRS_FORMAT_POS         5U
#define DS1307_HRS_FORMAT_MASK        (0x3U << DS1307_HRS_FORMAT_POS)

#define SUNDAY                       1U
#define MONDAY                       2U
#define TUESDAY                      3U
#define WEDNESDAY                    4U
#define THURSDAY                     5U
#define FRIDAY                       6U
#define SATURDAY                     7U

/*
 * DS1307 uses I2C1 on STM32G071.
 * PB8 = I2C1_SCL, PB9 = I2C1_SDA, AF6.
 */
#define DS1307_I2C                   I2C1
#define DS1307_I2C_GPIO_PORT         GPIOB
#define DS1307_I2C_SCL_PIN           GPIO_PIN_NO_8
#define DS1307_I2C_SDA_PIN           GPIO_PIN_NO_9
#define DS1307_I2C_PIN_AF            6U
#define DS1307_I2C_SPEED             I2C_SCL_SPEED_SM
#define DS1307_I2C_PUPD              GPIO_PU

/*
 * Date information.
 *
 * date  = day of month, 1 to 31
 * month = month, 1 to 12
 * year  = last two digits of year, 0 to 99
 * day   = day of week, 1 to 7
 */
typedef struct{
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t day;
} RTC_date_t;

/*
 * Time information.
 *
 * seconds     = 0 to 59
 * minutes     = 0 to 59
 * hours       = 0 to 23 in 24-hour mode, or 1 to 12 in 12-hour mode
 * time_format = TIME_FORMAT_12HRS_AM / TIME_FORMAT_12HRS_PM / TIME_FORMAT_24HRS
 */
typedef struct{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t time_format;
} RTC_time_t;

uint8_t ds1307_init(void);

void ds1307_set_current_time(RTC_time_t *rtc_time);
void ds1307_get_current_time(RTC_time_t *rtc_time);

void ds1307_set_current_date(RTC_date_t *rtc_date);
void ds1307_get_current_date(RTC_date_t *rtc_date);

#endif
