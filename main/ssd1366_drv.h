/// @brief The available fonts enumeration
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
} txtMsg;

/// @brief The ssd1306 device initialization
void ssd1306_init();

/// @brief Display the given text according with the given description
/// @param msg The structure that contains line number, font reference and text to be displayed
void ssd1306_display_text(txtMsg msg);

/// @brief Clear the whole screen to black
void ssd1306_clear();
