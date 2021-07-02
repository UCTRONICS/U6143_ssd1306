/*********************************************************************
* Demo for SSD1306 I2C driver for Raspberry Pi 
*********************************************************************/

#include <unistd.h>      // sleep, usleep
#include "ssd1306_i2c.h" // LCD_Display, ssd1306_begin

int main(void)
{
  unsigned short int count = 0;

  ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS); // LCD Screen initialization
  usleep(150000);                                           // Short delay to ensure the normal response of the lower functions

  // Loop forever
  for (;;)
  {
    LCD_Display(count);
    sleep(1);
    count++;
    if (count > 2)
    {
      count = 0;
    }
  }
}
