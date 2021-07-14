using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace OLED_Demo
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        SSD1306 OLED = new SSD1306();
        public MainPage()
        {
            this.InitializeComponent();
        }
        protected override async void OnNavigatedTo(NavigationEventArgs e)
        {
            await OLED.InitializeAsync();
            OLED.ShowString(0, 1, "Arducam", 8);
            OLED.ShowNum(0, 2, 12345678, 8, 8);
            DispatcherTimer timer = new DispatcherTimer
            {
                Interval = TimeSpan.FromSeconds(1)
            };
            timer.Tick += Timer_Tick;
            timer.Start();
        }
        private void Timer_Tick(object sender, object e)
        {
            string time = DateTime.Now.ToString("HH:mm:ss");
            for (int i = 0; i < time.Length; i++)
            {
                switch (time[i])
                {
                    case '0':
                        OLED.ShowChar(0 + 8 * i, 0, '0', 8);
                        break;
                    case '1':
                        OLED.ShowChar(0 + 8 * i, 0, '1', 8);
                        break;
                    case '2':
                        OLED.ShowChar(0 + 8 * i, 0, '2', 8);
                        break;
                    case '3':
                        OLED.ShowChar(0 + 8 * i, 0, '3', 8);
                        break;
                    case '4':
                        OLED.ShowChar(0 + 8 * i, 0, '4', 8);
                        break;
                    case '5':
                        OLED.ShowChar(0 + 8 * i, 0, '5', 8);
                        break;
                    case '6':
                        OLED.ShowChar(0 + 8 * i, 0, '6', 8);
                        break;
                    case '7':
                        OLED.ShowChar(0 + 8 * i, 0, '7', 8);
                        break;
                    case '8':
                        OLED.ShowChar(0 + 8 * i, 0, '8', 8);
                        break;
                    case '9':
                        OLED.ShowChar(0 + 8 * i, 0, '9', 8);
                        break;
                    case ':':
                        OLED.ShowChar(0 + 8 * i, 0, ':', 8);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
