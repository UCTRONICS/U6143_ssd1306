#include <stdio.h>
#include <fcntl.h>         // open
#include <errno.h>         // errno
#include <linux/i2c-dev.h> // I2C_SLAVE_FORCE
#include <stdlib.h>        // atof
#include <string.h>        // strcmp, strcpy, strtok
#include <sys/statfs.h>    // statfs
#include <sys/sysinfo.h>   // sysinfo
#include "ssd1306_i2c.h"
#include "bmp.h"
#include "oled_fonts.h"
#include <sys/types.h>
#include <sys/ioctl.h>     // ioctl
#include <net/if.h>        // if
#include <unistd.h>        // close, sleep, usleep
#include <arpa/inet.h>     // inet_ntoa

int i2cfd;

/*
* Init SSD1306
*/
void ssd1306_begin(unsigned int vccstate, unsigned int i2caddr)
{
  // I2C init
  i2cfd = open("/dev/i2c-1", O_RDWR);
  if (i2cfd < 0)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to initialize /dev/i2c-1: %s.\n", strerror(errno));
    exit(1);
  }

  if (ioctl(i2cfd, I2C_SLAVE_FORCE, i2caddr) < 0) {
    fprintf(stderr, "ssd1306_i2c: ioctl error: %s.\n", strerror(errno));
    exit(1);
  }

  OLED_WR_Byte(0xAE, OLED_CMD); // Disable display
  OLED_WR_Byte(0x40, OLED_CMD); // ---set low column address
  OLED_WR_Byte(0xB0, OLED_CMD); // ---set high column address
  OLED_WR_Byte(0xC8, OLED_CMD); // -not offset
  OLED_WR_Byte(0x81, OLED_CMD); // Set Contrast
  OLED_WR_Byte(0xff, OLED_CMD);
  OLED_WR_Byte(0xa1, OLED_CMD);
  OLED_WR_Byte(0xa6, OLED_CMD);
  OLED_WR_Byte(0xa8, OLED_CMD);
  OLED_WR_Byte(0x1f, OLED_CMD);
  OLED_WR_Byte(0xd3, OLED_CMD);
  OLED_WR_Byte(0x00, OLED_CMD);
  OLED_WR_Byte(0xd5, OLED_CMD);
  OLED_WR_Byte(0xf0, OLED_CMD);
  OLED_WR_Byte(0xd9, OLED_CMD);
  OLED_WR_Byte(0x22, OLED_CMD);
  OLED_WR_Byte(0xda, OLED_CMD);
  OLED_WR_Byte(0x02, OLED_CMD);
  OLED_WR_Byte(0xdb, OLED_CMD);
  OLED_WR_Byte(0x49, OLED_CMD);
  OLED_WR_Byte(0x8d, OLED_CMD);
  OLED_WR_Byte(0x14, OLED_CMD);
  OLED_WR_Byte(0xaf, OLED_CMD);
}

/*
* To send data
*/
void Write_IIC_Data(unsigned char IIC_Data)
{
  unsigned char msg[2] = {0x40, 0};

  msg[1] = IIC_Data;
  if (write(i2cfd, msg, 2) != 2) {
    fprintf(stderr, "ssd1306_i2c: Error writing data to /dev/i2c-1: %s.\n", strerror(errno));
  }
}

/*
* Send the command
*/
void Write_IIC_Command(unsigned char IIC_Command)
{
  unsigned char msg[2]={0x00, 0};

  msg[1] = IIC_Command;
  if (write(i2cfd, msg, 2) != 2) {
    fprintf(stderr, "ssd1306_i2c: Error writing command to /dev/i2c-1: %s.\n", strerror(errno));
  }
}

/*
* Write a byte
*/
void OLED_WR_Byte(unsigned int dat, unsigned int cmd)
{
  if (cmd)
  {
    Write_IIC_Data(dat);
  }
  else
  {
    Write_IIC_Command(dat);
  }

  usleep(500);
}

/*
* Coordinate setting
*/
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
  OLED_WR_Byte(0xb0 + y, OLED_CMD);
  OLED_WR_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);
  OLED_WR_Byte((x & 0x0f), OLED_CMD);
}

void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char chr, unsigned char Char_Size)
{
  unsigned char c = 0, i = 0;

  c = chr - ' '; // Get the offset value
  if (x > SSD1306_LCDWIDTH - 1)
  {
    x = 0;
    y = y + 2;
  }
  if (Char_Size == 16)
  {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++)
      OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
    OLED_Set_Pos(x, y + 1);
    for (i = 0; i < 8; i++)
      OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
  }

  else
  {
    OLED_Set_Pos(x, y);
    for (i = 0; i < 6; i++)
      OLED_WR_Byte(F6x8[c][i], OLED_DATA);
  }
}

unsigned int OLED_pow(unsigned char m, unsigned char n)
{
  unsigned int result = 1;

  while (n--)
    result *= m;
  return result;
}

/*
* According to digital
* x,y :Starting point coordinates
* num :Integer [0-4294967295]
* len :Number of digits
* size:The font size
* mode:0 - Fill mode; 1 - Stacking patterns
*/
void OLED_ShowNum(unsigned char x, unsigned char y, unsigned int num, unsigned char len, unsigned char size2)
{
  unsigned char t, temp;
  unsigned char enshow = 0;

  for (t = 0; t < len; t++)
  {
    temp = (num / OLED_pow(10, len - t - 1)) % 10;
    if (enshow == 0 && t < (len - 1))
    {
      if (temp == 0)
      {
        OLED_ShowChar(x + (size2 / 2) * t, y, ' ', size2);
        continue;
      }
      else
        enshow = 1;
    }
    OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0', size2);
  }
}

/*
* Displays a string of characters
*/
void OLED_ShowString(unsigned char x, unsigned char y, char *chr, unsigned char Char_Size)
{
  unsigned char j = 0;

  while (chr[j] != '\0')
  {
    OLED_ShowChar(x, y, chr[j], Char_Size);
    x += 8;
    if (x > 120)
    {
      x = 0;
      y += 2;
    }
    j++;
  }
}

/*
* Display the BMP image 128X32
* x,y:Starting point coordinates; x [0-127], y[0-4]
*/
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[][512], unsigned char symbol)
{
  unsigned int j = 0;
  unsigned char x, y;

  if (y1 % 8 == 0)
    y = y1 / 8;
  else
    y = y1 / 8 + 1;
  for (y = y0; y < y1; y++)
  {
    OLED_Set_Pos(x0, y);
    for (x = x0; x < x1; x++)
    {
      OLED_WR_Byte(BMP[symbol][j++], OLED_DATA);
    }
  }
}

void OLED_DrawPartBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[][512], unsigned char symbol)
{
  unsigned int j = x1 * y0;
  unsigned char x, y;

  if (y1 % 8 == 0)
    y = y1 / 8;
  else
    y = y1 / 8 + 1;
  for (y = y0; y < y1; y++)
  {
    OLED_Set_Pos(x0, y);
    for (x = x0; x < x1; x++)
    {
      OLED_WR_Byte(BMP[symbol][j++], OLED_DATA);
    }
  }
}

/*
* Clear specified row
*/
void OLED_ClearLint(unsigned char x1, unsigned char x2)
{
  unsigned char i, n;

  for (i = x1; i < x2; i++)
  {
    OLED_WR_Byte(0xb0 + i, OLED_CMD); // Set page address
    OLED_WR_Byte(0x00, OLED_CMD);     // Sets the display location - column low address
    OLED_WR_Byte(0x10, OLED_CMD);     // Sets the display location - column high address
    for (n = 0; n < 128; n++)
      OLED_WR_Byte(0, OLED_DATA);
  }
}

void OLED_Clear(void)
{
  unsigned char i, n;

  for (i = 0; i < 4; i++)
  {
    OLED_WR_Byte(0xb0 + i, OLED_CMD);
    OLED_WR_Byte(0x00, OLED_CMD);
    OLED_WR_Byte(0x10, OLED_CMD);
    for (n = 0; n < 128; n++)
      OLED_WR_Byte(0, OLED_DATA);
  }
}

/*
* sprintf-like format string with a variable number of decimals, but a fixed total number of characters
* len  :Number of characters to print
* len+1:Size of `dest`
*/
void sprintf_fix(char *dest, int len, float f)
{
  int prec = len - 1;
  float power10 = 1.0;

  while (f >= power10 && prec > 0)
  {
    power10 *= 10;
    prec--;
  }

  char buf[len + 2];
  snprintf(buf, sizeof buf, "%.*f", prec, f);
  if (buf[0] == '0')
  {
    strcpy(dest, buf + 1);
    return;
  }
  else
  {
    strncpy(dest, buf, (unsigned int)len);
    dest[len] = 0;
  }  

  // Result is shorter than width
  if (strlen(buf) < len)
  {
    // Right-align to field
    snprintf(dest, len + 1, "%*s", len, buf);
  }
}

/*
* Get CPU usage using popen(top / grep /awk)
*/
float GetCpuUsageTop(void)
{
  FILE *fp;
  char buffer[16] = {0};

  fp = popen("env CPULOOP=1 top -bn 1 | grep -m 1 'Cpu(s)' | awk '{print 100 - $8}'", "r");
  if (fp == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to query top for CPU usage.\n");
    return 0;
  }

  fgets(buffer, sizeof(buffer), fp);

  pclose(fp);

/*
  fprintf(stderr, "top CPU %f%%\n", atof(buffer));
*/

  return atof(buffer);
}

struct cpustat {
  unsigned long int t_user;
  unsigned long int t_nice;
  unsigned long int t_system;
  unsigned long int t_idle;
  unsigned long int t_iowait;
  unsigned long int t_irq;
  unsigned long int t_softirq;
  unsigned long int t_steal;
  unsigned long int t_guest;
  unsigned long int t_guestnice;
};

/*
* Get CPU usage using pstat
*/
float GetCpuUsagePstat(void)
{
  FILE *fd;
  struct cpustat prev, cur;

  fd = fopen("/proc/stat", "r");
  if(fd == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to open /proc/stat file.\n");
    return 0;
  }

  fscanf(fd, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
    &prev.t_user, &prev.t_nice, &prev.t_system, &prev.t_idle, &prev.t_iowait, &prev.t_irq, &prev.t_softirq, &prev.t_steal, &prev.t_guest, &prev.t_guestnice);

  fclose(fd);

  unsigned long long int prev_total = prev.t_idle + prev.t_iowait + prev.t_user + prev.t_nice + prev.t_system + prev.t_irq + prev.t_softirq + prev.t_steal;

/*
  fprintf(stderr, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", prev.t_user, prev.t_nice, prev.t_system, prev.t_idle, prev.t_iowait, prev.t_irq, prev.t_softirq, prev.t_steal, prev.t_guest, prev.t_guestnice);
  unsigned long long int prev_idle = prev.t_idle + prev.t_iowait;
  unsigned long long int prev_nonidle = prev.t_user + prev.t_nice + prev.t_system + prev.t_irq + prev.t_softirq + prev.t_steal;
  //unsigned long long int prev_total = prev_idle + prev_nonidle;
*/

  // Wait for one second to collect data to calculate delta
  sleep(3);

  fd = fopen("/proc/stat", "r");
  if(fd == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to open /proc/stat peudofile.\n");
    return 0;
  }

  fscanf(fd, "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
    &cur.t_user, &cur.t_nice, &cur.t_system, &cur.t_idle, &cur.t_iowait, &cur.t_irq, &cur.t_softirq, &cur.t_steal, &cur.t_guest, &cur.t_guestnice);

  fclose(fd);

  unsigned long long int cur_total = cur.t_idle + cur.t_iowait + cur.t_user + cur.t_nice + cur.t_system + cur.t_irq + cur.t_softirq + cur.t_steal;

/*
  fprintf(stderr, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\n", cur.t_user, cur.t_nice, cur.t_system, cur.t_idle, cur.t_iowait, cur.t_irq, cur.t_softirq, cur.t_steal, cur.t_guest, cur.t_guestnice);
  unsigned long long int cur_idle = cur.t_idle + cur.t_iowait;
  unsigned long long int cur_nonidle = cur.t_user + cur.t_nice + cur.t_system + cur.t_irq + cur.t_softirq + cur.t_steal;
  //unsigned long long int cur_total = cur_idle + cur_nonidle;

  fprintf(stderr, "PREV: idle:%llu\tnonidle: %llu\ttotal: %llu\n", prev_idle, prev_nonidle, prev_total);
  fprintf(stderr, "CUR: idle:%llu\tnonidle: %llu\ttotal: %llu\n", cur_idle, cur_nonidle, cur_total);

  // Calculate delta values
  unsigned long int total_d = cur_total - prev_total;
  unsigned long int idle_d = cur_idle - prev_idle;
  unsigned long int used_d = total_d - idle_d;
  //float cpu_perc = (float)used_d / (float)total_d * 100.0;
  fprintf(stderr, "TOTAL prev: %llu\tcurr: %llu\tdelta: %lu\n", prev_total, cur_total, total_d);
  fprintf(stderr, "IDLE  prev: %llu\tcurr: %llu\tdelta: %lu\n", prev_idle, cur_idle, idle_d);
  fprintf(stderr, "USED  delta: %lu\n", used_d);
*/
  
  float cpu_perc = (float)(cur_total - prev_total - cur.t_idle - cur.t_iowait + prev.t_idle + prev.t_iowait) / (float)(cur_total - prev_total) * 100.0;

/*
  fprintf(stderr, "pstat CPU %f%%\n", cpu_perc);
*/

  return cpu_perc;
}

char *GetIpAddress(void)
{
  FILE *fd;
  char line[100], *p, *c;

  fd = fopen("/proc/net/route", "r");
  if(fd == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to open /proc/net/route peudofile.\n");
    return '\0';
  }

  while (fgets(line, 100, fd))
  {
    p = strtok(line, " \t");
    c = strtok(NULL, " \t");

    if (p != NULL && c != NULL)
    {
      if (strcmp(c, "00000000") == 0)
      {
        break;
      }
    }
  }

  fclose(fd);

  // Default to eth0 interface
  if (p == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to determine default interface. Defaulting to eth0.\n");
    p = "eth0";
  }

  int sockfd;
  struct ifreq ifr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  // Type of address to retrieve - IPv4 IP address
  ifr.ifr_addr.sa_family = AF_INET;

  // Copy the interface name in the ifreq structure
  strncpy(ifr.ifr_name, p, IFNAMSIZ - 1);

  ioctl(sockfd, SIOCGIFADDR, &ifr);

  close(sockfd);

  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

float GetTemperature(void)
{
  FILE *fd;
  char buffer[150] = {0};
  unsigned int temp;

  fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
  if(fd == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to open /sys/class/thermal/thermal_zone0/temp peudofile.\n");
    return 0;
  }

  fgets(buffer, sizeof(buffer), fd);

  fclose(fd);

  // Parse buffer
  sscanf(buffer, "%u", &temp);

  return temp / 1000; // Temperature in degrees Celcius
}

/*
* LCD displays CPU temperature and other information
*/
void LCD_DisplayTemperature(void)
{
  char temp[3] = {0};
  char cpu_perc[5] = {0};
  char ip[16] = {0};

  sprintf_fix(temp, 2, GetTemperature());       // Gets the temperature of the CPU
  sprintf_fix(cpu_perc, 4, GetCpuUsagePstat()); // Gets the load on the CPU
  strcpy(ip, GetIpAddress());                   // Gets the IP address of the default interface
  OLED_Clear();                                 // Clear the screen
  OLED_DrawBMP(0, 0, 128, 4, BMP, 0);

  OLED_ShowString(8, 0, ip, 8);        // Display IP address
  OLED_ShowString(48, 3, temp, 8);     // Display CPU temperature
  OLED_ShowString(85, 3, cpu_perc, 8); // Display CPU load
}

/*
* LCD displays CPU memory and other information
*/
void LCD_DisplayCpuMemory(void)
{
  struct sysinfo s_info;
  char free[4] = {0};
  char total[4] = {0};

  if (sysinfo(&s_info) == 0) // Get memory information
  {
    OLED_ClearLint(2, 4);
    OLED_DrawPartBMP(0, 2, 128, 4, BMP, 1);

    float freeram = ((unsigned long long int)s_info.freeram * (unsigned long long int)s_info.mem_unit >> 20) / 1024.0;
    sprintf_fix(free, 3, freeram);
    float totalram = ((unsigned long long int)s_info.totalram * (unsigned long long int)s_info.mem_unit >> 20) / 1024.0;
    sprintf_fix(total, 3, totalram);

/*
    fprintf(stderr, "mem_unit: %u\n", s_info.mem_unit);
    fprintf(stderr, "RAM Free: %f GiB\tTotal: %f GiB\n",
      (float)(s_info.freeram * (unsigned long long)s_info.mem_unit >> 20) / 1024.0,
      (float)(s_info.totalram * (unsigned long long)s_info.mem_unit >> 20) / 1024.0);
*/

    OLED_ShowString(55, 3, free, 8);
    OLED_ShowString(90, 3, total, 8);
  }
  else
  {
    fprintf(stderr, "ssd1306_i2c: sysinfo() call failed.\n");
  }
}

/*
* LCD displays SD card memory information using popen(df / awk)
* (Better portability)
*/
void LCD_DisplaySdMemoryDf(void)
{
  char *mnt_dir = "/";
  FILE *fp;
  char buffer[32] = {0};
  float usedsize = 0;
  float totalsize = 0;
  char used[4] = {0};
  char total[4] = {0};

  fp = popen("df / | awk '{if (NR==2) {print $3 / 1048576\" \"$2 / 1048576}}'", "r");
  if (fp == NULL)
  {
    fprintf(stderr, "ssd1306_i2c: Unable to stat %s filesystem.\n", mnt_dir);
    return;
  }

  fgets(buffer, sizeof(buffer), fp);

  pclose(fp);

  // Parse buffer
  sscanf(buffer, "%f %f", &usedsize, &totalsize);
  sprintf_fix(total, 3, totalsize);
  sprintf_fix(used, 3, usedsize);

/*
  fprintf(stderr, "Disk Used: %f\tTotal: %f\n", usedsize, totalsize);
  fprintf(stderr, "Disk Used: %s GiB\tTotal: %s GiB\n", used, total);
*/

  OLED_ClearLint(2, 4);
  OLED_DrawPartBMP(0, 2, 128, 4, BMP, 2);

  OLED_ShowString(90, 3, total, 8);
  OLED_ShowString(55, 3, used, 8);
}

/*
* LCD displays SD card memory information using statfs()
*/
void LCD_DisplaySdMemoryStatfs(void)
{
  char *mnt_dir = "/";
  struct statfs fs;
  char used[4] = {0};
  char total[4] = {0};
  if (statfs(mnt_dir, &fs) == 0) // Get mounted filesystem information
  {
	float totalsize = (((unsigned long long int)fs.f_blocks * (unsigned long long int)fs.f_bsize) >> 20) / 1024.0;
    sprintf_fix(total, 3, totalsize);
    float usedsize = (((unsigned long long int)(fs.f_blocks - fs.f_bfree) * (unsigned long long int)fs.f_bsize) >> 20) / 1024.4;
    sprintf_fix(used, 3, usedsize);

/*
    fprintf(stderr, "f_bsize: %u\n", fs.f_bsize);
    fprintf(stderr, "Disk Used: %f GiB\tTotal: %f GiB\n", usedsize, totalsize);
    fprintf(stderr, "Disk Used: %s GiB\tTotal: %s GiB\n", used, total);
*/

    OLED_ClearLint(2, 4);
    OLED_DrawPartBMP(0, 2, 128, 4, BMP, 2);

    OLED_ShowString(90, 3, total, 8);
    OLED_ShowString(55, 3, used, 8);
  }
  else
  {
    fprintf(stderr, "ssd1306_i2c: Unable to stat %s filesystem.\n", mnt_dir);
  }
}

/*
* Display according to the information
*/
void LCD_Display(unsigned short int count)
{
  switch (count)
  {
  case 0:
    LCD_DisplayTemperature();
	sleep(3);
    break;
  case 1:
    LCD_DisplayCpuMemory();
	sleep(3);
    break;
  case 2:
    LCD_DisplaySdMemoryDf();
    break;
  default:
    break;
  }
}
