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

int p = 0;

void task_blink(void *ignore)
{
	while (1)
	{
		// printf("Turning the LED on\n");
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelay(1000 / portTICK_PERIOD_MS);

		// printf("Turning the LED off\n");
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay(500 / portTICK_PERIOD_MS);

		if (p == 3)
		{
			p = 0;
		}
		else
		{
			p += 3;
		}
	}

	vTaskDelete(NULL);
}

void task_count(void *ignore)
{
	int i = 25463;

	txtDescr txt16 = {1, "ABCDEFGH", F16x16};
	xTaskCreate(&task_ssd1306_display_text, "display_text2", 2048, (void *)&txt16, 6, NULL);

	while (1)
	{
		i++;
		int pg = p;
		char str[16];
		sprintf(str, "%d: %d", pg, i);

		txtDescr txt = {pg, str, F08x08};
		// txtDescr txt = {0, "-", F08x08};
		// txtDescr txt = {0, "+-", F16x16};
		// txtDescr txt2 = {1, "ABCD", F16x16};
		// txtDescr txt = {0, "A", F08x08};
		// txtDescr txt1 = {0, "-", F16x16};
		// txtDescr txt2 = {0, ",", F16x16};
		// txtDescr txt = {2, "-", F16x16};
		// txtDescr txt = {3, "-", F16x16};

		// xTaskCreate(&task_ssd1306_display_text, "display_text1", 2048, (void *)&txt1, 6, NULL);
		// xTaskCreate(&task_ssd1306_display_text, "display_text2", 2048, (void *)&txt2, 6, NULL);
		xTaskCreate(&task_ssd1306_display_text, "display_text2", 2048, (void *)&txt, 6, NULL);

		vTaskDelay(50 / portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
}

void app_init(void)
{
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

	xTaskCreate(task_ssd1306_display_clear, "clear", 2048, NULL, 6, NULL);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	xTaskCreate(&task_blink, "blink", 2048, NULL, 6, NULL);
	xTaskCreate(&task_count, "count", 2048, NULL, 6, NULL);
}

// https://github.com/Moddable-OpenSource/moddable/issues/244
// https://github.com/yanbe/ssd1306-esp-idf-i2c
// https://www.esp32.com/viewtopic.php?t=18656
