#include "nokia5110_LCD.h"

struct LCD_att lcd;
struct LCD_GPIO lcd_gpio;

// #region Set GPIO's, Init and config
/*----- GPIO Functions -----*/
/*
 * @brief Set functions for GPIO pins used
 * @param PORT: port of the pin used
 * @param PIN: pin of the pin used
 */
void LCD_setRST(GPIO_TypeDef *PORT, uint16_t PIN)
{
  lcd_gpio.RSTPORT = PORT;
  lcd_gpio.RSTPIN = PIN;
}

void LCD_setCE(GPIO_TypeDef *PORT, uint16_t PIN)
{
  lcd_gpio.CEPORT = PORT;
  lcd_gpio.CEPIN = PIN;
}

void LCD_setDC(GPIO_TypeDef *PORT, uint16_t PIN)
{
  lcd_gpio.DCPORT = PORT;
  lcd_gpio.DCPIN = PIN;
}
void LCD_setDIN(GPIO_TypeDef *PORT, uint16_t PIN)
{
  lcd_gpio.DINPORT = PORT;
  lcd_gpio.DINPIN = PIN;
}

void LCD_setCLK(GPIO_TypeDef *PORT, uint16_t PIN)
{
  lcd_gpio.CLKPORT = PORT;
  lcd_gpio.CLKPIN = PIN;
}

/*
 * @brief Initializes the LCD using predetermined values.
 * GPIO's must be set before calling this function.
 */
void LCD_init()
{
  HAL_GPIO_WritePin(lcd_gpio.RSTPORT, lcd_gpio.RSTPIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(lcd_gpio.RSTPORT, lcd_gpio.RSTPIN, GPIO_PIN_SET);
  LCD_send(0x21, LCD_COMMAND);               //LCD extended commands.
  LCD_send(0xCC, LCD_COMMAND);               //set LCD Vop(Contrast).
  LCD_send(0x04, LCD_COMMAND);               //set temp coefficent.
  LCD_send(0x14, LCD_COMMAND);               //LCD bias mode 1:40.
  LCD_send(0x20, LCD_COMMAND);               //LCD basic commands.
  LCD_send(LCD_DISPLAY_NORMAL, LCD_COMMAND); //LCD normal.
  LCD_clrScr();
  lcd.inverttext = false;
}

/*
 * @brief Inverts the colour shown on the display
 * @param mode: true = inverted / false = normal
 */
void LCD_invert(bool mode)
{
  if (mode == true)
  {
    LCD_send(LCD_DISPLAY_INVERTED, LCD_COMMAND);
  }
  else
  {
    LCD_send(LCD_DISPLAY_NORMAL, LCD_COMMAND);
  }
}

/*
 * @brief Inverts the colour of any text sent to the display
 * @param mode: true = inverted / false = normal
 */
void LCD_invertText(bool mode)
{
  if (mode == true)
  {
    lcd.inverttext = true;
  }
  else
  {
    lcd.inverttext = false;
  }
}
// #endregion

// #region Low-level SPI commands
/*
 * @brief Sends data to the LCD using configured GPIOs
 * @param data: data to be sent
 * @param mode: command or data
 */
void LCD_send(uint8_t data, uint8_t mode)
{
  if (mode == LCD_COMMAND)
  {
    HAL_GPIO_WritePin(lcd_gpio.DCPORT, lcd_gpio.DCPIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lcd_gpio.CEPORT, lcd_gpio.CEPIN, GPIO_PIN_RESET);
    _LCD_send(data);
    HAL_GPIO_WritePin(lcd_gpio.CEPORT, lcd_gpio.CEPIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(lcd_gpio.DCPORT, lcd_gpio.DCPIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(lcd_gpio.CEPORT, lcd_gpio.CEPIN, GPIO_PIN_RESET);
    _LCD_send(data);
    HAL_GPIO_WritePin(lcd_gpio.CEPORT, lcd_gpio.CEPIN, GPIO_PIN_SET);
  }
}

/*
 * @brief Sends data to the LCD using configured GPIOs
 * @param data: data to be sent
 */
void _LCD_send(uint8_t data)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    HAL_GPIO_WritePin(lcd_gpio.DINPORT, lcd_gpio.DINPIN, !!(data & (1 << (7 - i))));
    HAL_GPIO_WritePin(lcd_gpio.CLKPORT, lcd_gpio.CLKPIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(lcd_gpio.CLKPORT, lcd_gpio.CLKPIN, GPIO_PIN_RESET);
  }
}
// #endregion

// #region Clear, Refresh and Move commands
/*
 * @brief Clears the screen
 */
void LCD_clrScr()
{
  for (int i = 0; i < 504; i++)
  {
    LCD_send(0x00, LCD_DATA);
    lcd.buffer[i] = 0;
  }
}

/*
 * @brief Sets LCD's cursor to position (x, row)
 * @param x: position in px on the x-axis (column)
 * @param row: row number
 */
void LCD_goXY(uint8_t x, uint8_t row)
{
  LCD_send(LCD_SETXADDR | x, LCD_COMMAND); // x
  LCD_send(LCD_SETYADDR | row, LCD_COMMAND); // row
}

/*
 * @brief Updates the entire screen according to lcd.buffer
 */
void LCD_refreshScr()
{
  LCD_goXY(LCD_SETXADDR, LCD_SETYADDR);
  for (int i = 0; i < 6; i++)
  {
    for (int j = 0; j < LCD_WIDTH; j++)
    {
      LCD_send(lcd.buffer[(i * LCD_WIDTH) + j], LCD_DATA);
    }
  }
}

/*
 * @brief Updates a square of the screen according to given values
 * @param xmin: starting point on the x-axis
 * @param xmax: ending point on the x-axis
 * @param ymin: starting point on the y-axis
 * @param ymax: ending point on the y-axis
 */
void LCD_refreshArea(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax)
{
  for (int row = 0; row < 6; row++)
  {
    if (row * 8 > ymax)
    {
      break;
    }
    LCD_send(LCD_SETYADDR | row, LCD_COMMAND);
    LCD_send(LCD_SETXADDR | xmin, LCD_COMMAND);
    for (int x = xmin; x <= xmax; x++)
    {
      LCD_send(lcd.buffer[(row * LCD_WIDTH) + x], LCD_DATA);
    }
  }
}
// #endregion

// #region Print text commands
/*
 * @brief Prints a string on the LCD, with a default font
 * @param x: starting point on the x-axis (column)
 * @param row: starting row
 */
void LCD_print(char *str, uint8_t x, uint8_t row)
{
  LCD_goXY(x, row);
  while (*str)
  {
    LCD_printChar(*str++);
  }
}

/**
 * @brief Prints a string on the LCD, with the given font
 * @param x: starting point on the x-axis (column)
 * @param row: starting row
 * @param font: the font to use from font.h
 * */
void LCD_printWithFont(char *str, uint8_t x, uint8_t row, const uint8_t *font)
{
  Position next_position;
  next_position.x = x;
  next_position.row = row;
  next_position.error = false;
  while (*str && !next_position.error)
  {
    next_position = LCD_printCharWithFont(*str++, next_position.x, next_position.row, font);
  }
}

/*
 * @brief Prints one char on the current position of LCD's cursor, with a default font.
 * @param c: char to be printed
 */
void LCD_printChar(char c)
{
  uint8_t *font = Ascii6_8;
  uint8_t font_x_size = font[FONT_HEADER_X_SIZE_INDEX];
  uint8_t font_y_size = font[FONT_HEADER_Y_SIZE_INDEX];
  uint8_t font_ascii_offset = font[FONT_HEADER_ASCII_OFFSET_INDEX];
  uint8_t font_chars_count = font[FONT_HEADER_CHARS_COUNT_INDEX];
  // Screen is 84px (width, 0 <= x < 84) * 48px (height, 0 <= y < 48)
  // height (48px) is divided in 6 rows (0 <= row < 6) of 8 vertical pixels (1 byte).
  // Some fonts spread on several lines
  uint8_t font_rows_count = font_y_size / 8;

  uint16_t char_index = FONT_HEADER_BYTES + ((c - font_ascii_offset) * (font_x_size * font_rows_count));

  for (int i = 0; i < 6; i++)
  {
    if (lcd.inverttext != true)
      LCD_send(Ascii6_8[char_index + i], LCD_DATA);
    else
      LCD_send(~(Ascii6_8[char_index + i]), LCD_DATA);
  }
}

/**
 * Prints one char to LCD at the given position with the given font.
 * 
 * @param c: the char to write
 * @retval the position for the next character.
 */
Position LCD_printCharWithFont(unsigned char c, int x, int row, uint8_t *font)
{
  Position next_position;

  uint8_t font_x_size = font[FONT_HEADER_X_SIZE_INDEX];
  uint8_t font_y_size = font[FONT_HEADER_Y_SIZE_INDEX];
  uint8_t font_ascii_offset = font[FONT_HEADER_ASCII_OFFSET_INDEX];
  uint8_t font_chars_count = font[FONT_HEADER_CHARS_COUNT_INDEX];
  // Screen is 84px (width, 0 <= x < 84) * 48px (height, 0 <= y < 48)
  // height (48px) is divided in 6 rows (0 <= row < 6) of 8 vertical pixels (1 byte).
  // Some fonts spread on several lines
  uint8_t font_rows_count = font_y_size / 8;

  if (x + font_x_size > LCD_WIDTH || row + font_rows_count >= LCD_ROWS_COUNT)
  {
    // ERROR: out of screen
    next_position.error = true;
    return next_position;
  }

  uint16_t char_index = 0;      // char index in font array
  uint16_t char_part_index = 0; // part of char index in font array

  for (int row_offset = 0; row_offset < font_rows_count; row_offset++)
  {
    // Move the y-address pointer for every pointer.
    LCD_send(LCD_SETYADDR | (row + row_offset), LCD_COMMAND);
    // Move to the x-address.
    LCD_send(LCD_SETXADDR | x, LCD_COMMAND);

    char_index = FONT_HEADER_BYTES + ((c - font_ascii_offset) * (font_x_size * font_rows_count));

    for (uint16_t x_offset = 0; x_offset < font_x_size; x_offset++)
    {
      char_part_index = char_index + x_offset + (row_offset * font_x_size);
      char s = font[char_part_index];
      if (!lcd.inverttext)
      {
        LCD_send(s, LCD_DATA);
      }
      else
      {
        LCD_send(~s, LCD_DATA);
      }
    }
  }

  // Reset
  LCD_send(LCD_SETYADDR, LCD_COMMAND);
  LCD_send(LCD_SETXADDR, LCD_COMMAND);

  // Return position for next char [x, row]
  if (x + font_x_size <= LCD_WIDTH - font_x_size)
  {
    next_position.x = x + font_x_size;
    next_position.row = row;
  }
  else
  {
    next_position.x = 0;
    next_position.row = row + font_rows_count;
  }
  return next_position;
}
// #endregion

// #region Draw commands
/*
 * @brief Sets a pixel on the screen
 */
void LCD_setPixel(uint8_t x, uint8_t y, bool pixel)
{
  if (x >= LCD_WIDTH)
    x = LCD_WIDTH - 1;
  if (y >= LCD_HEIGHT)
    y = LCD_HEIGHT - 1;

  if (pixel != false)
  {
    lcd.buffer[x + (y / 8) * LCD_WIDTH] |= 1 << (y % 8);
  }
  else
  {
    lcd.buffer[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
  }
}

/*
 * @brief Draws a horizontal line
 * @param x: starting point on the x-axis (px)
 * @param y: starting point on the y-axis (px)
 * @param l: length of the line
 */
void LCD_drawHLine(int x, int y, int l)
{
  int by, bi;

  if ((x >= 0) && (x < LCD_WIDTH) && (y >= 0) && (y < LCD_HEIGHT))
  {
    for (int cx = 0; cx < l; cx++)
    {
      by = ((y / 8) * LCD_WIDTH) + x;
      bi = y % 8;
      lcd.buffer[by + cx] |= (1 << bi);
    }
  }
}

/*
 * @brief Draws a vertical line
 * @param x: starting point on the x-axis (px)
 * @param y: starting point on the y-axis (px)
 * @param l: length of the line
 */
void LCD_drawVLine(int x, int y, int l)
{

  if ((x >= 0) && (x < LCD_WIDTH) && (y >= 0) && (y < LCD_HEIGHT))
  {
    for (int cy = 0; cy <= l; cy++)
    {
      LCD_setPixel(x, y + cy, true);
    }
  }
}

/*
 * @brief abs function used in LCD_drawLine
 * @param x: any integer
 * @return absolute value of x
 */
int abs(int x)
{
  if (x < 0)
  {
    return x * -1;
  }
  return x;
}

/*
 * @brief Draws any line
 * @param x1: starting point on the x-axis (px)
 * @param y1: starting point on the y-axis (px)
 * @param x2: ending point on the x-axis (px)
 * @param y2: ending point on the y-axis (px)
 */
void LCD_drawLine(int x1, int y1, int x2, int y2)
{
  int tmp;
  double delta, tx, ty;

  if (((x2 - x1) < 0))
  {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }
  if (((y2 - y1) < 0))
  {
    tmp = x1;
    x1 = x2;
    x2 = tmp;
    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  if (y1 == y2)
  {
    if (x1 > x2)
    {
      tmp = x1;
      x1 = x2;
      x2 = tmp;
    }
    LCD_drawHLine(x1, y1, x2 - x1);
  }
  else if (x1 == x2)
  {
    if (y1 > y2)
    {
      tmp = y1;
      y1 = y2;
      y2 = tmp;
    }
    LCD_drawHLine(x1, y1, y2 - y1);
  }
  else if (abs(x2 - x1) > abs(y2 - y1))
  {
    delta = ((double)(y2 - y1) / (double)(x2 - x1));
    ty = (double)y1;
    if (x1 > x2)
    {
      for (int i = x1; i >= x2; i--)
      {
        LCD_setPixel(i, (int)(ty + 0.5), true);
        ty = ty - delta;
      }
    }
    else
    {
      for (int i = x1; i <= x2; i++)
      {
        LCD_setPixel(i, (int)(ty + 0.5), true);
        ty = ty + delta;
      }
    }
  }
  else
  {
    delta = ((float)(x2 - x1) / (float)(y2 - y1));
    tx = (float)(x1);
    if (y1 > y2)
    {
      for (int i = y2 + 1; i > y1; i--)
      {
        LCD_setPixel((int)(tx + 0.5), i, true);
        tx = tx + delta;
      }
    }
    else
    {
      for (int i = y1; i < y2 + 1; i++)
      {
        LCD_setPixel((int)(tx + 0.5), i, true);
        tx = tx + delta;
      }
    }
  }
}

/*
 * @brief Draws a rectangle
 * @param x1: starting point on the x-axis (px)
 * @param y1: starting point on the y-axis (px)
 * @param x2: ending point on the x-axis (px)
 * @param y2: ending point on the y-axis (px)
 */
void LCD_drawRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
  LCD_drawLine(x1, y1, x2, y1);
  LCD_drawLine(x1, y1, x1, y2);
  LCD_drawLine(x2, y1, x2, y2);
  LCD_drawLine(x1, y2, x2, y2);
}
// #endregion
