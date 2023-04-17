#pragma once

#ifndef TIMES
#define TIMES

#include <time.h>

#include "ssd1366_drv.h"

#include "times.h"

// https://github.com/hwstar/freertos-avr/blob/master/include/time.h

void times_init();
void times_display();
void times_increment_second();

#endif /* TIMES */
