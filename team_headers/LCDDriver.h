//LCDDriver.h

//C2000 doesn't have an 8-bit type, just use the unsigned 16-bit int
#define uint8_t unsigned int
#define int8_t int
#define uint16_t unsigned int

//LCD utility functions
extern void LCD_reset(void);
extern void LCDinit (void);
extern void clear_screen(uint8_t option);
extern void LCDSplash(uint16_t ms);

//Text functions
extern void set_cursor(uint8_t x, int8_t y);
extern void print_cstr(const uint8_t* str, int8_t inv);
extern void clear_to_end();			//clears a box 1 font height tall from the current cursor position to the edge of the screen
extern void print_char(uint8_t txt, uint8_t inv);
extern void del_char();
extern void set_font(const uint8_t *pNewFont);

//Graphics functions
extern void pixel(uint8_t S_R, uint8_t x, uint8_t y);
extern void line(uint8_t S_R, int8_t x0, int8_t y0, int8_t x1, int8_t y1);
extern void circle(uint8_t S_R, int8_t x0, int8_t y0, int8_t r);
extern void box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
extern void draw_block(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data);
extern void draw_sprite(uint8_t x, uint8_t y, uint8_t n, uint8_t mode);
extern void bitblt(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t* data);

//access to fonts
extern const uint8_t Font[];





