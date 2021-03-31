# U6143_ssd1306
## Preparation
```bash
sudo raspi-config
```
Choose Interface Options 
Enable i2c

##  Clone U6143_ssd1306 library 
```bash
git clone https://github.com/UCTRONICS/U6143_ssd1306.git
```
## Compile 
```bash
cd U6143_ssd1306/C
```
```bash
sudo make clean && sudo make 
```
## Run 
```
sudo ./display
```

## Add automatic start script
- Open the rc.local file 
```bash
sudo nano /etc/rc.local
```
- Add command to the rc.local file
```bash
cd /home/pi/U6143_ssd1306/C
sudo make clean 
sudo make 
sudo ./display &
```
- reboot your system








