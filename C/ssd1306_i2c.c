#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/vfs.h>
#include "ssd1306_i2c.h"
#include "bmp.h"
#include "oled_fonts.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>


char IPSource[20]={0};
int i2cd;

// Init SSD1306
void ssd1306_begin(unsigned int vccstate, unsigned int i2caddr)
{
  unsigned char count=0;
  FILE* fp;
  unsigned char buffer[20]={0};
  unsigned char i2c[20]="/dev/i2c-1";
  // I2C Init
  i2cd = open(i2c,O_RDWR);
  if (i2cd < 0) 
  {
	  fprintf(stderr, "Device I2C-1 failed to initialize\n");
	  return;
  }
 if (ioctl(i2cd, I2C_SLAVE_FORCE, i2caddr) < 0)
  {
    return;
  }
	OLED_WR_Byte(0xAE,OLED_CMD);//Disable display
	OLED_WR_Byte(0x40,OLED_CMD);//---set low column address
	OLED_WR_Byte(0xB0,OLED_CMD);//---set high column address
	OLED_WR_Byte(0xC8,OLED_CMD);//-not offset
	OLED_WR_Byte(0x81,OLED_CMD);//Set Contrast
	OLED_WR_Byte(0xff,OLED_CMD);
	OLED_WR_Byte(0xa1,OLED_CMD);
	OLED_WR_Byte(0xa6,OLED_CMD);
	OLED_WR_Byte(0xa8,OLED_CMD);
	OLED_WR_Byte(0x1f,OLED_CMD);
	OLED_WR_Byte(0xd3,OLED_CMD);
	OLED_WR_Byte(0x00,OLED_CMD);
	OLED_WR_Byte(0xd5,OLED_CMD);
	OLED_WR_Byte(0xf0,OLED_CMD);
	OLED_WR_Byte(0xd9,OLED_CMD);
	OLED_WR_Byte(0x22,OLED_CMD);
	OLED_WR_Byte(0xda,OLED_CMD);
	OLED_WR_Byte(0x02,OLED_CMD);
	OLED_WR_Byte(0xdb,OLED_CMD);
	OLED_WR_Byte(0x49,OLED_CMD);
	OLED_WR_Byte(0x8d,OLED_CMD);
	OLED_WR_Byte(0x14,OLED_CMD);
	OLED_WR_Byte(0xaf,OLED_CMD); 
}





//Displays a string of characters
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr,unsigned char Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j],Char_Size);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}

void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char Char_Size)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';    //Get the offset value	
		if(x>SSD1306_LCDWIDTH-1){x=0;y=y+2;}
		if(Char_Size ==16)
		{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
		}

		else {	
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
				OLED_WR_Byte(F6x8[c][i],OLED_DATA);
				
			}
}

unsigned int oled_pow(unsigned char m,unsigned char n)
{
	unsigned int result=1;	 
	while(n--)result*=m;    
	return result;
}	

//According to digital
//x,y :Starting point coordinates	 
//len :Number of digits
//size:The font size
//mode:	0,Fill mode;1,Stacking patterns
//num:(0~4294967295);	 		  
void OLED_ShowNum(unsigned char x,unsigned char y,unsigned int num,unsigned char len,unsigned char size2)
{         	
	unsigned char t,temp;
	unsigned char enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 


//Coordinate setting
void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f),OLED_CMD); 
} 

//Write a byte
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{

	if(cmd)
	{
		Write_IIC_Data(dat);
	}
	else 
	{
		Write_IIC_Command(dat);
	}

  usleep(500); 
}

//To send data
void Write_IIC_Data(unsigned char IIC_Data)
{
  unsigned char msg[2]={0x40,0};
  msg[1]=IIC_Data;
  write(i2cd, msg, 2);
}

//Send the command
void Write_IIC_Command(unsigned char IIC_Command)
{
  unsigned char msg[2]={0x00,0};
  msg[1]=IIC_Command;
  write(i2cd, msg, 2);
}


/***********Display the BMP image  128X32  Starting point coordinates(x,y),The range of x 0~127   The range of y 0~4*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[][512],unsigned char symbol)
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
		for(x=x0;x<x1;x++)
		{      
			OLED_WR_Byte(BMP[symbol][j++],OLED_DATA);	    	
		}
	}
} 


void OLED_DrawPartBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[][512],unsigned char symbol)
{ 	
 unsigned int j=x1*y0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
		for(x=x0;x<x1;x++)
		{      
			OLED_WR_Byte(BMP[symbol][j++],OLED_DATA);	    	
		}
	}
} 

/*
*	Clear specified row
*/
void OLED_ClearLint(unsigned char x1,unsigned char x2)
{
	unsigned char i,n;		    
	for(i=x1;i<x2;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //Set page address
		OLED_WR_Byte (0x00,OLED_CMD);      //Sets the display location - column low address
		OLED_WR_Byte (0x10,OLED_CMD);      //Sets the display location - column high address 
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} 
}	

void OLED_Clear(void)  
{  
	unsigned char i,n;		    
	for(i=0;i<4;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);  
		OLED_WR_Byte (0x00,OLED_CMD);      
		OLED_WR_Byte (0x10,OLED_CMD);     
		for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA); 
	} 
}

/*
*    LCD displays CPU temperature and other information
*/
void LCD_DisplayTemperature(void)
{
  unsigned char symbol=0;
  unsigned int temp=0;
  FILE * fp;
  unsigned char  buffer[80] = {0};
  temp=Obaintemperature();                  //Gets the temperature of the CPU
  fp=popen("top -bn1 | grep load | awk '{printf \"%.2f\", $(NF-2)}'","r");    //Gets the load on the CPU
  fgets(buffer, sizeof (buffer),fp);                                    //Read the CPU load
  pclose(fp);
  buffer[3]='\0';        
  
  OLED_Clear();                                        //Remove the interface
  OLED_DrawBMP(0,0,128,4,BMP,TEMPERATURE_TYPE);
  if (IP_SWITCH == IP_DISPLAY_OPEN)
  {
    strcpy(IPSource,GetIpAddress());   //Get the IP address of the device's wireless network card
    OLED_ShowString(0,0,IPSource,8);          //Send the IP address to the lower machine
  }
  else
  {
    OLED_ShowString(0,0,CUSTOM_DISPLAY,8);          //Send the IP address to the lower machine
  }

  if(temp>=100)                                                  
  {
    OLED_ShowChar(50,3,temp/100+'0',8);                        //According to the temperature
    OLED_ShowChar(58,3,temp/10%10+'0',8);                        //According to the temperature
    OLED_ShowChar(66,3,temp%10+'0',8);  
  }
  else if(temp<100&&temp>=10)   
  {
    OLED_ShowChar(58,3,temp/10+'0',8);                        //According to the temperature
    OLED_ShowChar(66,3,temp%10+'0',8);  
  }
  else
  {
    OLED_ShowChar(66,3,temp+'0',8);
  }
  OLED_ShowString(87,3,buffer,8);                        //Display CPU load
}

unsigned char Obaintemperature(void)
{
    FILE *fd;
    unsigned int temp;
    char buff[150] = {0};
    fd = fopen("/sys/class/thermal/thermal_zone0/temp","r");
    fgets(buff,sizeof(buff),fd);
    sscanf(buff, "%d", &temp);
    fclose(fd);
    return TEMPERATURE_TYPE == FAHRENHEIT ? temp/1000*1.8+32 : temp/1000;

}

/*
*  LCD displays CPU memory and other information
*/
void LCD_DisPlayCpuMemory(void)
{
  struct sysinfo s_info;
  float Totalram=0.0;
  float freeram=0.0;
  unsigned int value=0;
  unsigned char Total[10]={0};
  unsigned char free[10]={0};
  unsigned char buffer[100]={0};
  unsigned char famer[100]={0};
  if(sysinfo(&s_info)==0)            //Get memory information
  {
    OLED_ClearLint(2,4);
    OLED_DrawPartBMP(0,2,128,4,BMP,2);
    FILE* fp=fopen("/proc/meminfo","r");
    if(fp==NULL)
    {
      return ;
    }
    while(fgets(buffer,sizeof(buffer),fp))
    {
      if(sscanf(buffer,"%s%u",famer,&value)!=2)
      {
	continue;
      }
      if(strcmp(famer,"MemTotal:")==0)
      {
	Totalram=value/1024.0/1024.0;
      }
      else if(strcmp(famer,"MemFree:")==0)
      {
	freeram=value/1024.0/1024.0;
      }
    }
    fclose(fp);
    Total[0]=(unsigned char)Totalram+'0';
    Total[1]='.';
    Total[2]=((unsigned char)(Totalram*10))%10+'0';
    Total[3]=='\0';
    
    free[0]=(unsigned char)freeram+'0';
    free[1]='.';
    free[2]=((unsigned char)(freeram*10))%10+'0';
    free[3]=='\0';
    OLED_ShowString(55,3,free,8); 
    OLED_ShowString(90,3,Total,8); 
  }
}


/*
*  LCD displays SD card memory information
*/
void LCD_DisplaySdMemory(void)
{
  char usedsize_GB[10]={0};
  char totalsize_GB[10]={0};
  unsigned int MemSize=0;
  unsigned int size=0;
  struct statfs diskInfo;
  statfs("/",&diskInfo);
  OLED_ClearLint(2,4);
  OLED_DrawPartBMP(0,2,128,4,BMP,3);  
  unsigned long long blocksize = diskInfo.f_bsize;// The number of bytes per block
  unsigned long long totalsize = blocksize*diskInfo.f_blocks;//Total number of bytes	
  MemSize=(unsigned int)(totalsize>>30);
  snprintf(totalsize_GB,7,"%d",MemSize);
  if(MemSize>0&&MemSize<10)
  {
    OLED_ShowString(106,3,totalsize_GB,8); 
  }
  else if(MemSize>=10&&MemSize<100)
  {
    OLED_ShowString(98,3,totalsize_GB,8);     
  }
  else
  {
    OLED_ShowString(90,3,totalsize_GB,8); 
  }


	unsigned long long freesize = blocksize*diskInfo.f_bfree; //Now let's figure out how much space we have left
  size=freesize>>30;
  size=MemSize-size;
  snprintf(usedsize_GB,7,"%d",size);
  if(size>0&&size<10)
  {
    OLED_ShowString(65,3,usedsize_GB,8); 
  }
  else if(size>=10&&size<100)
  {
    OLED_ShowString(58,3,usedsize_GB,8);     
  }
  else
  {
    OLED_ShowString(55,3,usedsize_GB,8); 
  }
}

/*
*According to the information
*/
void LCD_Display(unsigned char symbol)
{
  switch(symbol)
  {
    case 0:
      LCD_DisplayTemperature();
    break;
    case 1:
      LCD_DisPlayCpuMemory();
    break;
    case 2:
      LCD_DisplaySdMemory();
    break;
    default:
    break;
  }
}


void FirstGetIpAddress(void)
{
  strcpy(IPSource,GetIpAddress());     
}

char* GetIpAddress(void)
{
    int fd;
    struct ifreq ifr;
    int symbol=0;
    if (IPADDRESS_TYPE == ETH0_ADDRESS)
    {
      fd = socket(AF_INET, SOCK_DGRAM, 0);
      /* I want to get an IPv4 IP address */
      ifr.ifr_addr.sa_family = AF_INET;
      /* I want IP address attached to "eth0" */
      strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
      symbol=ioctl(fd, SIOCGIFADDR, &ifr);
      close(fd);
      if(symbol==0)
      {
        return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
      }
      else
      {
        char* buffer="0.0.0.0";
        return buffer;
      }
    }
    else if (IPADDRESS_TYPE == WLAN0_ADDRESS)
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        /* I want to get an IPv4 IP address */
        ifr.ifr_addr.sa_family = AF_INET;
        /* I want IP address attached to "wlan0" */
        strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);
        symbol=ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);    
        if(symbol==0)
        {
          return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);   
        }
        else
        {
          char* buffer="0.0.0.0";
          return buffer;
        }
    }
    else
    {
      char* buffer="0.0.0.0";
      return buffer;
    }
}
