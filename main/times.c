#include <time.h>
#include <stdio.h>
#include <string.h>

#include "times.h"

time_t rawtime = 0;
txtMsg main_time;
txtMsg main_date;

/// @brief The short names ob the day weeks
char *day[] = {
	"Th",
	"Fr",
	"Sa",
	"Su",
	"Mo",
	"Tu",
	"We",
}; // crazy week of day numbers

/// @brief The short names ob the months
char *mon[] = {
	"---",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

/// @brief Sets initial date and time
void times_init()
{
	char str_date[17];
	char str_time[9];

	main_time.line = 0;
	main_time.font = F16x16;
	main_time.text = str_time;

	main_date.line = 3;
	main_date.font = F08x08;
	main_date.text = str_date;

	struct tm ptm;
	rawtime = time(NULL);
	struct tm *ptm2 = localtime(&rawtime);
	memcpy(&ptm, ptm2, sizeof(struct tm));

	ptm.tm_year = 2023;
	ptm.tm_mon = 4;
	ptm.tm_mday = 19;
	ptm.tm_hour = 23;
	ptm.tm_min = 59;
	ptm.tm_sec = 48;
	// ptm.tm_wday = 2; // crazy week of day numbers

	rawtime = mktime(&ptm);
	// set_system_time(mktime(ptm)); // It doesn't exists with espressif
}

/// @brief Increments time
void times_increment_second()
{
	rawtime++;
}

/// @brief Displays the main screen
void times_display()
{
	struct tm ptm;
	struct tm *ptm2 = localtime(&rawtime);
	memcpy(&ptm, ptm2, sizeof(struct tm));

	sprintf(main_time.text, "%02d:%02d:%02d",
			ptm.tm_hour, ptm.tm_min, ptm.tm_sec);

	sprintf(main_date.text, "%s, %s %02d, %04d",
			day[ptm.tm_wday], mon[ptm.tm_mon], ptm.tm_mday, ptm.tm_year);

	ssd1306_display_text(main_time);
	ssd1306_display_text(main_date);
}
