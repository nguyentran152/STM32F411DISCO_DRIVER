/*
 * rtc.c
 *
 *  Created on: Mar 14, 2023
 *      Author: Nguyen Tran
 */

#include <stdio.h>
#include "ds1307.h"
#include "lcd.h"


#define SYSTICK_TIM_CLK   16000000UL

#define PRINT_LCD

char* get_day_of_week(uint8_t i);
void number_to_string(uint8_t num , char* buf);
char* time_to_string(RTC_time_t *rtc_time);
char* date_to_string(RTC_date_t *rtc_date);
void init_systick_timer(uint32_t tick_hz);


static void mdelay(uint32_t cnt)
{
	for(uint32_t i=0 ; i < (cnt * 1000); i++);
}


int main()
{
	RTC_time_t current_time;
	RTC_date_t current_date;
	char *am_pm;

#ifndef PRINT_LCD

	printf("RTC testing\n");

#else
	lcd_init();

	display_msg("RTC testing...", 1, 1);

	mdelay(2000);

	display_clear();
	lcd_display_return_home();
#endif

	if(ds1307_init()){
#ifndef PRINT_LCD
		printf("RTC init failed\n");
#else
		display_msg("RTC init failed", 1, 1);
		display_clear();
		mdelay(2000);
#endif
		while(1);
	}

	init_systick_timer(1);

	current_date.day 		= WEDNESDAY;
	current_date.date 		= 15;
	current_date.month 		= 3;
	current_date.year		= 23;

	current_time.hours		= 17;
	current_time.minutes	= 14;
	current_time.seconds	= 45;
	current_time.time_format= TIME_FORMAT_24_HRS;

	ds1307_set_current_date(&current_date);
	ds1307_set_current_time(&current_time);

	ds1307_get_current_time(&current_time);
	ds1307_get_current_date(&current_date);


	if(current_time.time_format != TIME_FORMAT_24_HRS)
	{
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(current_time),am_pm);
#else
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif
	}else
	{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(current_time));
#else
		lcd_print_string(time_to_string(&current_time));
#endif
	}
#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(current_date), get_day_of_week(current_date->day)); //15/03/2023 <Wednesday>
#else
	display_msg(date_to_string(&current_date), 2, 1);
#endif
	while(1);
	return 0;
}


char* get_day_of_week(uint8_t i)
{
	char* days[] = { "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

	return days[i-1];
}


void number_to_string(uint8_t num , char* buf)
{

	if(num < 10){
		buf[0] = '0';
		buf[1] = num + 48;
	}else if(num >= 10 && num < 99)
	{
		buf[0] = (num/10) + 48;
		buf[1]= (num % 10) + 48;
	}
}


//hh:mm:ss
char* time_to_string(RTC_time_t *rtc_time)
{
	static char buf[9];

	buf[2]= ':';
	buf[5]= ':';

	number_to_string(rtc_time->hours, buf);
	number_to_string(rtc_time->minutes,&buf[3]);
	number_to_string(rtc_time->seconds,&buf[6]);

	buf[8] = '\0';

	return buf;
}




//dd/mm/yy
char* date_to_string(RTC_date_t *rtc_date)
{
	static char buf[9];

	buf[2]= '/';
	buf[5]= '/';

	number_to_string(rtc_date->date,buf);
	number_to_string(rtc_date->month,&buf[3]);
	number_to_string(rtc_date->year,&buf[6]);

	buf[8]= '\0';

	return buf;

}



void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

    /* calculation of reload value */
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;

    //Clear the value of SVR
    *pSRVR &= ~(0x00FFFFFFFF);

    //load the value in to SVR
    *pSRVR |= count_value;

    //do some settings
    *pSCSR |= ( 1 << 1); //Enables SysTick exception request:
    *pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source

    //enable the systick
    *pSCSR |= ( 1 << 0); //enables the counter

}


void SysTick_Handler(void)
{
	RTC_time_t current_time;
	RTC_date_t current_date;
	char *am_pm;

	ds1307_get_current_time(&current_time);
	if(current_time.time_format != TIME_FORMAT_24_HRS)
	{
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); //01:25:45 AM
#else
		display_msg(time_to_string(&current_time), 1, 1);
		lcd_print_string(am_pm);
#endif

	}else
	{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time));
#else
		display_msg(time_to_string(&current_time), 1, 1);
#endif
	}

	ds1307_get_current_date(&current_date);

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day)); //15/03/2023 <tuesday>
#else
	display_msg(date_to_string(&current_date), 2, 1);
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
#endif

}
