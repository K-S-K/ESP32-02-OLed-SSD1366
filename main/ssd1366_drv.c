#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"

#include "sdkconfig.h"

#include "ssd1366.h"
#include "ssd1366_drv.h"
#include "font8x8.h"

#define tag "SSD1306"

void i2c_master_init()
{
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 1000000};
    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void ssd1306_init()
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true); // The Charge Pump must be enabled by the following command:
    i2c_master_write_byte(cmd, 0x14, true);                     // 14h - Enable Charge Pump
                                                                // 8Dh - Charge Pump Setting
                                                                // AFh - Display ON

    i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
    i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    if (espRc == ESP_OK)
    {
        ESP_LOGI(tag, "OLED configured successfully");
    }
    else
    {
        ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
    }
    i2c_cmd_link_delete(cmd);
}

void task_ssd1306_display_pattern(void *ignore)
{
    i2c_cmd_handle_t cmd;

    for (uint8_t i = 0; i < 8; i++)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
        i2c_master_write_byte(cmd, 0xB0 | i, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
        for (uint8_t j = 0; j < OLED_CONFIG_PIXEL_H; j++)
        {
            i2c_master_write_byte(cmd, 0xFF >> (j % 8), true);
        }
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }

    vTaskDelete(NULL);
}

void ssd1306_display_drawsub_line(uint8_t line, uint8_t *data)
{
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
    i2c_master_write_byte(cmd, 0xB0 | line, true); // B0~B7 Set GDDRAM Page Start Address (PAGE0~PAGE7) for Page Addressing Mode

    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    i2c_master_write(cmd, data, OLED_CONFIG_PIXEL_H, true);

    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
}

uint8_t extract(uint8_t input, bool up)
{
    bool dn = !up;
    uint8_t output = 0;

    if (dn)
    {
        if (input & 0b00010000)
        {
            output |= 0b00000010;
        }
        if (input & 0b00100000)
        {
            output |= 0b00001000;
        }
        if (input & 0b01000000)
        {
            output |= 0b00100000;
        }
        if (input & 0b10000000)
        {
            output |= 0b10000000;
        }
    }
    if (up)
    {
        if (input & 0b00000001)
        {
            output |= 0b00000010;
        }
        if (input & 0b00000010)
        {
            output |= 0b00001000;
        }
        if (input & 0b00000100)
        {
            output |= 0b00100000;
        }
        if (input & 0b00001000)
        {
            output |= 0b10000000;
        }
    }

    return output;
}

void ssd1306_display_drawline(uint8_t line, uint8_t *data)
{
    uint8_t dataUp[OLED_CONFIG_PIXEL_H];
    uint8_t dataDn[OLED_CONFIG_PIXEL_H];

    for (int i = 0; i < OLED_CONFIG_PIXEL_H; i++)
    {
        dataUp[i] = extract(data[i], true);
        dataDn[i] = extract(data[i], false);
    }

    ssd1306_display_drawsub_line(line * 2, dataUp);
    ssd1306_display_drawsub_line(line * 2 + 1, dataDn);
}

void task_ssd1306_display_clear(void *ignore)
{
    uint8_t zero[OLED_CONFIG_PIXEL_H];
    memset(zero, 0x00, sizeof(zero));

    for (uint8_t line = 0; line < 4; line++)
    {
        ssd1306_display_drawline(line, zero);
    }

    vTaskDelete(NULL);
}

void task_ssd1306_contrast(void *ignore)
{
    i2c_cmd_handle_t cmd;

    uint8_t contrast = 0;
    uint8_t direction = 1;
    while (true)
    {
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
        i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
        i2c_master_write_byte(cmd, contrast, true);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        vTaskDelay(1 / portTICK_PERIOD_MS);

        contrast += direction;
        if (contrast == 0xFF)
        {
            direction = -1;
        }
        if (contrast == 0x0)
        {
            direction = 1;
        }
    }
    vTaskDelete(NULL);
}

void task_ssd1306_display_text(const txtDescr *txt)
{
    int line = txt->line;
    char *text = txt->text;
    uint8_t text_len = strlen(text);
    uint8_t data[OLED_CONFIG_PIXEL_H];
    memset(data, 0x00, sizeof(data));

    for (uint8_t i = 0; i < text_len; i++)
    {
        int startIndex = i * 8;

        if (startIndex + 8 > OLED_CONFIG_PIXEL_H)
        {
            break;
        }

        memcpy(data + startIndex, font8x8[(uint8_t)text[i]], 8);
    }

    ssd1306_display_drawline(line, data);

    vTaskDelete(NULL);
}
