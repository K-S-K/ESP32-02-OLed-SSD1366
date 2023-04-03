typedef struct
{
    int line;
    char *text;
} txtDescr;

void i2c_master_init();
void ssd1306_init();

void task_ssd1306_contrast(void *ignore);
void task_ssd1306_display_clear(void *ignore);

void task_ssd1306_display_pattern(void *ignore);

void task_ssd1306_display_text(const txtDescr *txt);