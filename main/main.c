// idf.py build
// idf.py flash -p COM3 monitor
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "hardware_def.h"
#include "hardware_drv.h"
#include "ssd1366_drv.h"
#include "times.h"

void task_seconds(void *ignore)
{
	while (1)
	{
		// printf("Turning the LED on\n");
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelay(100 / portTICK_PERIOD_MS);

		// printf("Turning the LED off\n");
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay(900 / portTICK_PERIOD_MS);

		times_increment_second();
		times_display();
	}

	vTaskDelete(NULL);
}

void app_init(void)
{
	times_init();

	i2c_master_init();
	ssd1306_init();

	// Configure the IOMUX register for pad BLINK_GPIO
	// (some pads are muxed to GPIO on reset already,
	// but some default to other functions
	// and need to be switched to GPIO).
	gpio_reset_pin(BLINK_GPIO);

	// Set the GPIO as a push/pull output
	gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
	app_init();

	ssd1306_clear();

	xTaskCreate(&task_seconds, "seconds", 2048, NULL, 6, NULL);
}
