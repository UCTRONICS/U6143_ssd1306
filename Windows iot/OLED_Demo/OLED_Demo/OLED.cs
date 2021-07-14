
using System;
using System.Diagnostics;
using Windows.Devices.Enumeration;
using System.Threading.Tasks;
using Windows.Devices.I2c;

namespace OLED_Demo
{
    class SSD1306 : IDisposable
    {
        CharactersTable table = new CharactersTable();
        private const UInt32 SCREEN_WIDTH_PX = 128;                         /* Number of horizontal pixels on the display */
        private const UInt32 SCREEN_HEIGHT_PX = 32;                         /* Number of vertical pixels on the display   */
        private I2cDevice sensor;
        const string I2CControllerName = "I2C1";
        const byte OLED_ADDR = 0x3C;
        public async Task InitializeAsync()
        {
            var settings = new I2cConnectionSettings(OLED_ADDR);
            settings.BusSpeed = I2cBusSpeed.StandardMode;
            var controller = await I2cController.GetDefaultAsync();
            sensor = controller.GetDevice(settings);
            InitCommand();
            ClearScreen();
        }


        public void ShowString(int x, int y, string chr, byte Char_Size)
        {
            for (int i = 0; i < chr.Length; i++)
            {
                ShowChar(x, y, chr[i], Char_Size);
                x += 8;
                if (x > SCREEN_WIDTH_PX-8)
                {
                    x = 0;
                    y += 2;
                }
            }
        }
        public void ClearScreen()
        {
            for (int i = 0; i < 4; i++)
            {
                WriteCommand((byte)(0xb0 + i));
                WriteCommand(0x00);
                WriteCommand(0x10);
                for (int n = 0; n < SCREEN_WIDTH_PX; n++) WriteData(0);
            }
        }
        public void ShowChar(int x, int y, char chr, byte Char_Size)
        {
            int i = 0;
            int index = chr-' ';
            if (x > SCREEN_WIDTH_PX - 1) { x = 0; y = y + 2; }
            if (Char_Size == 16)
            {
                SetPoint(x, y);
                for (i = 0; i < 8; i++)
                {
                    WriteData(table.Table16x8[index * 16 + i]);
                }
                SetPoint(x, y + 1);
                for (i = 0; i < 8; i++)
                {
                    WriteData(table.Table16x8[index * 16 + i + 8]);
                }
            }
            else
            {
                SetPoint(x, y);
                for (i = 0; i < 6; i++)
                {
                    WriteData(table.Table6x8[index*6+i]);
                }
                    

            }
        }

        public void DrawBMP(int x0, int y0, int x1, int y1, byte[] buffer)
        {
            int j = 0;
            int x, y;
            for (y=y0; y < y1; y++)
            {
                SetPoint(x0, y);
                for (x = x0; x < x1; x++)
                {
                    WriteData(buffer[j++]);
                }
            }
        }

        public int oled_pow(int m, int n)
        {
            int result = 1;
            for (int i = 0; i < n; i++)
            {
                result *= m;
            }
            return result;
        }

        public void ShowNum(int x, int y, int num, byte len, byte Char_Size)
        {
            int t, temp;
            int enshow = 0;
            for (t = 0; t < len; t++)
            {
                temp = (num / oled_pow(10, len - t - 1)) % 10;
                if (enshow == 0 && t < (len - 1))
                {
                    if (temp == 0)
                    {
                        if (Char_Size == 16)
                        {
                            ShowChar(x + (Char_Size / 2) * t, y, ' ', Char_Size);
                        }
                        else
                        {
                            ShowChar(x + 8 * t, y, ' ', Char_Size);
                        }
                        continue;
                    }
                    else enshow = 1;

                }
                if (Char_Size == 16)
                {
                    ShowChar(x + (Char_Size / 2) * t, y, (char)(temp + 48), Char_Size);
                }
                else
                {
                    ShowChar(x + 8 * t, y, (char)(temp + 48), Char_Size);
                }
            }
        }

        public void WriteCommand(byte command)
        {
            sensor.Write(new byte[] { 0x00, command });
        }

        public void WriteData(byte data)
        {
            sensor.Write(new byte[] { 0x40, data });
        }


        public void SetPoint(int x, int y)
        {
            WriteCommand((byte)(0xb0 + y));
            WriteCommand((byte)(((x & 0xf0) >> 4) | 0x10));
            WriteCommand((byte)((x & 0x0f) | 0x01));
        }



        public I2cDevice GetDevice()
        {
            return sensor;
        }

        public void Dispose()
        {
            sensor.Dispose();
        }

        private void InitCommand()
        { 
            WriteCommand(0xAE);//Disable display
            WriteCommand(0x40);//---set low column address
            WriteCommand(0xB0);//---set high column address
            WriteCommand(0xC8);//-not offset
            WriteCommand(0x81);//Set Contrast
            WriteCommand(0xff);
            WriteCommand(0xa1);
            WriteCommand(0xa6);
            WriteCommand(0xa8);
            WriteCommand(0x1f);
            WriteCommand(0xd3);
            WriteCommand(0x00);
            WriteCommand(0xd5);
            WriteCommand(0xf0);
            WriteCommand(0xd9);
            WriteCommand(0x22);
            WriteCommand(0xda);
            WriteCommand(0x02);
            WriteCommand(0xdb);
            WriteCommand(0x49);
            WriteCommand(0x8d);
            WriteCommand(0x14);
            WriteCommand(0xaf);
        }
    }
}
