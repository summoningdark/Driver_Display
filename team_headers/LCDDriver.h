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
extern void print_char(uint8_t txt);
extern void del_char();
extern void set_font(uint8_t *pNewFont);

//Graphics functions
extern void pixel(uint8_t S_R, uint8_t x, uint8_t y);
extern void line(uint8_t S_R, int8_t x0, int8_t y0, int8_t x1, int8_t y1);
extern void circle(uint8_t S_R, int8_t x0, int8_t y0, int8_t r);
extern void box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
extern void draw_block(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t data);
extern void draw_sprite(uint8_t x, uint8_t y, uint8_t n, uint8_t mode);
extern void bitblt(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t mode, uint8_t* data);







