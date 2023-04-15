typedef enum
{
    F08x08,
    F16x16
} tFont;

/// @brief This structure represents a text which can be drawn on the screen
typedef struct
{
    /// @brief The logical number of the string
    int line;

    /// @brief The null terminated string of text which can be drawn
    char *text;

    /// @brief The enumerated value that explains choosen font
    tFont font;
} txtDescr;

/// @brief The ssd1306 device initialization
void ssd1306_init();

/// @brief Display the given text according with the given description
/// @param data The structure that contains line number, font reference and text to be displayed
void task_ssd1306_display_text(void *data);

/// @brief Clear the whole screen to black
/// @param ignore 
void task_ssd1306_display_clear(void *ignore);
