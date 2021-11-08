/*********************************************************************
*This is UCTRONICS ssd1306 library header file;
This library just support 128X32 lcd with i2c interface;
*********************************************************************/
#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

extern int i2cd;

/**********Select display temperature type**************/
#define CELSIUS       0
#define FAHRENHEIT    1
#define TEMPERATURE_TYPE  FAHRENHEIT
/**********Select display temperature type**************/

/**********Select display network IP type**************/
#define ETH0_ADDRESS    0
#define WLAN0_ADDRESS   1
#define IPADDRESS_TYPE  ETH0_ADDRESS
/**********Select display network IP type**************/


/************************Turn off the IP display. Can customize the display****************/
#define IP_DISPLAY_OPEN     0
#define IP_DISPLAY_CLOSE    1
#define IP_SWITCH       IP_DISPLAY_OPEN
#define CUSTOM_DISPLAY   "UCTRONICS"
/************************Turn off the IP display. Can customize the display****************/

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define OLED_CMD  0	                 //command
#define OLED_DATA 1	                //data
#define SSD1306_I2C_ADDRESS   0x3C	// 011110+SA0+RW - 0x0C    
#define SSD1306_SWITCHCAPVCC 0x2

#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 32
void ssd1306_begin(unsigned int switchvcc, unsigned int i2caddr); //switchvcc should be SSD1306_SWITCHCAPVCC
void OLED_ShowString(unsigned char x,unsigned char y, unsigned char *p,unsigned char Char_Size);
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char Char_Size);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_WR_Byte(unsigned dat,unsigned cmd);  
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Command(unsigned char IIC_Command);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[][512],unsigned char);
void OLED_DrawPartBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[][512],unsigned char);
void OLED_Clear(void);
void OLED_ClearLint(unsigned char,unsigned char);
void OLED_ShowNum(unsigned char,unsigned char,unsigned int,unsigned char,unsigned char);
unsigned int oled_pow(unsigned char,unsigned char);
void LCD_Display(unsigned char symbol);
void LCD_DisPlayCpuMemory(void);
void LCD_DisplaySdMemory(void);
void FirstGetIpAddress(void);
char* GetIpAddress(void);
unsigned char Obaintemperature(void);
#endif				/* _SSD1306_I2C_H_ */
