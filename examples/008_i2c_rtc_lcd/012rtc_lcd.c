#include <stdint.h>
#include "ds1307.h"
#include "lcd.h"

static void number_to_string(uint8_t number, char *buffer);
static char *time_to_string(RTC_time_t *rtc_time);
static char *date_to_string(RTC_date_t *rtc_date);
static char *get_day_of_week(uint8_t day);
static void init_systick_timer(uint32_t tick_hz);
void SysTick_Handler(void);

static RTC_time_t current_time;
static RTC_date_t current_date;

static volatile uint8_t rtc_update_pending = 0U;

static char *time_string;
static char *date_string;
static char *day_string;

int main(void)
{
    uint8_t init_status = 0U;

    RTC_time_t rtc_time;
    RTC_date_t rtc_date;

    rtc_time.seconds = 0U;
    rtc_time.minutes = 47U;
    rtc_time.hours = 13U;
    rtc_time.time_format = DS1307_TIME_FORMAT_24HRS;

    rtc_date.date = 14U;
    rtc_date.month = 6U;
    rtc_date.year = 26U;
    rtc_date.day = SUNDAY;

    init_status = ds1307_init();

    if (init_status != 0U){
        while (1);
    }

    lcd_init();

    /*
     * Set RTC time/date only once when needed.
     * Keep these lines commented during normal run so the RTC keeps counting.
     */

    //ds1307_set_current_time(&rtc_time);
    //ds1307_set_current_date(&rtc_date);

    init_systick_timer(1U);
    while (1){
    	ds1307_get_current_time(&current_time);
    	ds1307_get_current_date(&current_date);

    	time_string = time_to_string(&current_time);
    	date_string = date_to_string(&current_date);

    	lcd_set_cursor(0U, 0U);
    	lcd_send_string(time_string);

    	lcd_set_cursor(1U, 0U);
    	lcd_send_string(date_string);
    }
}

/*
 * Convert a number in the range 0-99 to a two-character decimal string.
 * Example: 5 -> "05", 14 -> "14".
 */
static void number_to_string(uint8_t number, char *buffer){
	if (number < 10U){
		buffer[0] = '0';
		buffer[1] = (char)(number + '0');
	}
	else{
		buffer[0] = (char)((number / 10U) + '0');
		buffer[1] = (char)((number % 10U) + '0');
	}
}

/* Format RTC time as HH:MM:SS */
static char *time_to_string(RTC_time_t *rtc_time){
	static char buffer[9];

	number_to_string(rtc_time->hours, &buffer[0]);
	buffer[2] = ':';

	number_to_string(rtc_time->minutes, &buffer[3]);
	buffer[5] = ':';

	number_to_string(rtc_time->seconds, &buffer[6]);
	buffer[8] = '\0';

	return buffer;
}

/* Format RTC date as DD/MM/YY */
static char *date_to_string(RTC_date_t *rtc_date){
	static char buffer[9];

	number_to_string(rtc_date->date, &buffer[0]);
	buffer[2] = '/';

	number_to_string(rtc_date->month, &buffer[3]);
	buffer[5] = '/';

	number_to_string(rtc_date->year, &buffer[6]);
	buffer[8] = '\0';

	return buffer;
}

static char *get_day_of_week(uint8_t day){
    static char *days[] = {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday"
    };

    if ((day < SUNDAY) || (day > SATURDAY)){
        return "Invalid";
    }

    return days[day - 1U];
}

static void init_systick_timer(uint32_t tick_hz){
    uint32_t reload_value = 0U;

    /*
     * Default system clock is HSI16 = 16 MHz.
     * SysTick interrupt frequency = tick_hz.
     */
    reload_value = (16000000U / tick_hz) - 1U;

    SysTick->LOAD = reload_value;
    SysTick->VAL = 0U;

    /*
     * CLKSOURCE = processor clock
     * TICKINT   = enable SysTick interrupt
     * ENABLE    = start SysTick
     */
    SysTick->CTRL = (1U << 2) | (1U << 1) | (1U << 0);
}

void SysTick_Handler(void){
    rtc_update_pending = 1U;
}
