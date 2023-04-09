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
#include "font08x08.h"
#include "font16x16.h"

#define tag "SSD1306"

// https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    ((byte)&0x80 ? '*' : '.'),     \
        ((byte)&0x40 ? '*' : '.'), \
        ((byte)&0x20 ? '*' : '.'), \
        ((byte)&0x10 ? '*' : '.'), \
        ((byte)&0x08 ? '*' : '.'), \
        ((byte)&0x04 ? '*' : '.'), \
        ((byte)&0x02 ? '*' : '.'), \
        ((byte)&0x01 ? '*' : '.')

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
        for (uint8_t j = 0; j < OLED_DISPLAY_WIDTH_PX; j++)
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
    i2c_master_write(cmd, data, OLED_DISPLAY_WIDTH_PX, true);

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

/// @brief Draw byte map
/// @param data
/// @param font_width
void report_bytes(uint8_t *data, int font_width)
{
    for (uint8_t i = 0, j = 0, l = 0; i < OLED_DISPLAY_WIDTH_PX; i++, j++)
    {
        if (j >= font_width)
        {
            printf("\n");
            j = 0;
        }

        if (j == 0)
        {
            // printf("i:%3d - %3d: ", i, i + font_width);
            printf("i:%2d: ", l++);
        }

        printf(" 0x%02x", data[i]);
    }

    printf("\n\n");
}

void ssd1306_display_drawline(uint8_t line, uint8_t *data)
{
    uint8_t dataUp[OLED_DISPLAY_WIDTH_PX];
    uint8_t dataDn[OLED_DISPLAY_WIDTH_PX];

    for (int i = 0; i < OLED_DISPLAY_WIDTH_PX; i++)
    {
        dataUp[i] = extract(data[i], true);
        dataDn[i] = extract(data[i], false);
    }

    ssd1306_display_drawsub_line(line * 2, dataUp);
    ssd1306_display_drawsub_line(line * 2 + 1, dataDn);
}

void task_ssd1306_display_clear(void *ignore)
{
    uint8_t zero[OLED_DISPLAY_WIDTH_PX];
    memset(zero, 0x00, sizeof(zero));

    for (uint8_t line = 0; line < 8; line++)
    {
        ssd1306_display_drawsub_line(line, zero);
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

void task_ssd1306_display_text(const txtDescr *txt) // TODO guard against multiline text
{
    bool enable_console_output = false;
    int target_page = txt->line;
    char *text = txt->text;
    tFont font = txt->font;
    uint8_t font_size = 0;
    uint8_t font_width = 0;
    uint8_t font_height = 0;
    switch (font)
    {
    case F08x08:
        if (enable_console_output)
            printf("F08x08 - 1\n");
        font_width = font08x08_width;
        font_size = font08x08_size;
        font_height = font08x08_height;
        break;

    case F16x16:
        if (enable_console_output)
            printf("font 16x16 - 1\n");
        font_width = font16x16_width;
        font_size = font16x16_size;
        font_height = font16x16_height;
        break;

    default:
        printf("No such font while getting size and width&height: got %d\n", font);
        vTaskDelete(NULL);
        return;
        break;
    }

    uint16_t text_len = strlen(text);
    uint16_t allowed_text_len = OLED_DISPLAY_WIDTH_PX / font_width;

    if (text_len > allowed_text_len)
    {
        text[allowed_text_len] = 0;
        text_len = strlen(text);
        printf("Text is too long, string will be cut to: '%s', length=%d\n", text, text_len);
    }

    uint8_t font_height_in_pages = font_height / OLED_PAGE_HEIGHT_PX;
    uint16_t picture_size = OLED_DISPLAY_WIDTH_PX * font_height_in_pages; // picture size will be 128 (bytes) if font takes 1 page, and 256 (bytes) if font takes 2 pages

    uint8_t img_data[picture_size];
    memset(img_data, 0x00, sizeof(img_data));

    if (enable_console_output)
        printf("target_page = %d; font_height_in_pages = %d; font_height = %d; font_size = %d; picture_size=%d; sizeof(img_data)=%d\n",
               target_page, font_height_in_pages, font_height, font_size, picture_size, sizeof(img_data));

    for (uint16_t i = 0; i < text_len; i++)
    {
        // printf("symbol[%d]: %c\n", i, text[i]);

        uint16_t startIndex = i * font_size;

        switch (font)
        {
        case F08x08:
            memcpy(img_data + startIndex, font08x08_data[(uint8_t)text[i]], font_size);
            if (enable_console_output)
                printf("font 08x08 - symbol c:%c(%d) is copied\n", text[i], (uint8_t)text[i]);
            break;

        case F16x16:
            memcpy(img_data + startIndex, font16x16_data[(uint8_t)text[i]], font_size);
            if (enable_console_output)
                printf("font 16x16 - symbol c:%c(%d) is copied\n", text[i], (uint8_t)text[i]);
            break;

        default:
            printf("No such font while copying images from font: got %d\n", font);
            vTaskDelete(NULL);
            return;
            break;
        }
    }

    if (enable_console_output)
        printf("Copying of images from font complete\n");

    /*
    // draw bit map
    // for (int i = 0; i < OLED_DISPLAY_WIDTH_PX / 8; i++)
    printf("The bit map for img_data\n");
    for (int i = 0; i < 64 / 8; i += font_size)
    {
        for (int j = 0; j < font_size; j++)
        {
            printf(" " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(img_data[i * 8 + j]));
        }
        printf("\n");
    }
    //*/

    if (font_height_in_pages == 1)
    {
        ssd1306_display_drawline(target_page, img_data);
        // report_bytes(img_data, font_width);
    }
    else
    {
        if (font_height_in_pages == 2)
        {
            // report_bytes(img_data, font_width);
            for (uint8_t p = 0; p < font_height_in_pages; p++)
            {
                // printf("page_number=%d\n", p);

                    int symbols_in_line = OLED_DISPLAY_WIDTH_PX / font_height;
                uint8_t page_data[OLED_DISPLAY_WIDTH_PX];
                memset(page_data, 0x00, sizeof(page_data));

                for (uint8_t i = 0; i < symbols_in_line; i++)
                {
                    int startIndex_dst = i * 8;
                    int startIndex_src = i * font_height + p * 8;

                    memcpy(page_data + startIndex_dst, img_data + startIndex_src, 8);
                }

                if (enable_console_output)
                    printf("send page_data to line=%d;\n", target_page + p);

                // Send 8-bit height line of the image to the screen
                ssd1306_display_drawline(target_page + p, page_data);
                // report_bytes(page_data, font_width);

                if (enable_console_output)
                    printf("page_data sent; line=%d;\n", target_page + p);
            }
        }
    }

    vTaskDelete(NULL);
}
