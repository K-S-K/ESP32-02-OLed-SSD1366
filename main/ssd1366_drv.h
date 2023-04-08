
typedef enum { F08x08, F16x16 } tFont;

typedef struct
{
    int line;
    char *text;
    tFont font;
} txtDescr;

void i2c_master_init();
void ssd1306_init();

void task_ssd1306_contrast(void *ignore);
void task_ssd1306_display_clear(void *ignore);

void task_ssd1306_display_pattern(void *ignore);

void task_ssd1306_display_text(const txtDescr *txt);