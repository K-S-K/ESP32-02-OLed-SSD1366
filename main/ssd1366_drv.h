
void i2c_master_init();
void ssd1306_init();

void task_ssd1306_display_pattern(void *ignore);
void ssd1306_display_drawsub_line(uint8_t line, uint8_t *data);
void ssd1306_display_drawline(uint8_t line, uint8_t *data);
void task_ssd1306_display_clear(void *ignore);

void task_ssd1306_contrast(void *ignore);
void task_ssd1306_scroll(void *ignore);

void task_ssd1306_display_string(const void *arg_text);
void task_ssd1306_display_text(const void *arg_text);