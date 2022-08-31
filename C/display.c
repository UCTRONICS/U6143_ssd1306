/*********************************************************************
 * Demo for SSD1306 I2C driver for Raspberry Pi
 *********************************************************************/

#include <signal.h>
#include <stdlib.h>      // exit
#include <unistd.h>      // sleep, usleep
#include "ssd1306_i2c.h" // LCD_Display, ssd1306_begin

void sig_handler(int signum) // Return type of the handler function should be void
{
  switch (signum)
  {
  case SIGHUP:
    OLED_Clear();
    exit(0);
  case SIGINT:
    OLED_Clear();
    exit(0);
  case SIGTERM:
    OLED_Clear();
    exit(0);
  default:
    break;
  }
}

int main(void)
{
  unsigned short int count = 0;

  signal(SIGHUP, sig_handler);  // Register signal handler (can't catch SIGKILL or SIGSTOP)
  signal(SIGINT, sig_handler);  // Register signal handler (can't catch SIGKILL or SIGSTOP)
  signal(SIGTERM, sig_handler); // Register signal handler (can't catch SIGKILL or SIGSTOP)

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
