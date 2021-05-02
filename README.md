# U6143_ssd1306
- Enable I2C interface
```bash
sudo raspi-config
```
Choose `Interface Options` | `I2C`
Answer `Yes` to whether you would like the ARM I2C interface to be enabled.
- Install Git and library dependencies
```bash
sudo apt update
sudo apt install git wiringpi
```

##  Clone U6143_ssd1306 library 
```bash
git clone https://github.com/UCTRONICS/U6143_ssd1306.git
```

## Compile 
```bash
cd U6143_ssd1306/C
```
```bash
make clean && make 
```

## Run 
```
sudo ./display
```

## Add automatic start script
- Copy binary to `/usr/local/bin/`
```bash
sudo cp ./display /usr/local/bin/
```
- Open the rc.local file 
```bash
sudo nano /etc/rc.local
```
- Add command to the rc.local file
```bash
/usr/local/bin/display &
```
## Reboot your system
```bash
sudo reboot now
```

## For older 0.91 inch lcd without mcu 
- For the older version lcd without mcu controller, you can use python demo
- Install the dependent library files
```bash
sudo pip3 install adafruit-circuitpython-ssd1306
sudo apt-get install python3-pip
sudo apt-get install python3-pil
```
- Test demo 
```bash 
cd /home/pi/U6143_ssd1306/python 
sudo python3 ssd1306_stats.py
```
