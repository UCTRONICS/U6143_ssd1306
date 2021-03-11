/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/

#include "ssd1306_i2c.h"
#include "time.h"



void main(void) 
{
  unsigned char symbol=0;
  ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);      //LCD Screen initialization
  delay(150);                                                  //Short delay Ensure the normal response of the lower function
  FirstGetIpAddress();                        //Get IP address
	while(1)
	{
    LCD_Display(symbol);
    sleep(1);
    sleep(1);
    sleep(1);
    symbol++;
    if(symbol==3)
    {
      symbol=0;
    }
	}
}
