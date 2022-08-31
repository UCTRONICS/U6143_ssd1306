/*********************************************************************
 * This is the UCTRONICS ssd1306 library header file
 * This library just supports the 128X32 LCD with I2C interface
 *********************************************************************/

#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

extern int i2cd;

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define OLED_CMD 0               // command
#define OLED_DATA 1              // data
#define SSD1306_I2C_ADDRESS 0x3C // 011110+SA0+RW - 0x0C
#define SSD1306_SWITCHCAPVCC 0x2

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 32

void ssd1306_begin(unsigned int switchvcc, unsigned int i2caddr); // switchvcc should be SSD1306_SWITCHCAPVCC
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Command(unsigned char IIC_Command);
void OLED_WR_Byte(unsigned int dat, unsigned int cmd);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char Char_Size);
unsigned int OLED_pow(unsigned char, unsigned char);
void OLED_ShowNum(unsigned char, unsigned char, unsigned int, unsigned char, unsigned char);
void OLED_ShowString(unsigned char x, unsigned char y, char *p, unsigned char Char_Size);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[][512], unsigned char);
void OLED_DrawPartBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[][512], unsigned char);
void OLED_ClearLint(unsigned char, unsigned char);
void OLED_Clear(void);
void sprintf_fix(char *dest, int len, float f);
float GetCpuUsageTop(void);
float GetCpuUsagePstat(void);
char *GetIpAddress(void);
float GetTemperature(void);
void LCD_DisplayTemperature(void);
void LCD_DisplayCpuMemory(void);
void LCD_DisplaySdMemoryDf(void);
void LCD_DisplaySdMemoryStatfs(void);
void LCD_Display(unsigned short int symbol);

#endif /* _SSD1306_I2C_H_ */
