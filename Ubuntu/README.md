# U6144_SSD1306

## C code
- Environmental configuration
```
sudo apt install make
```
```
sudo apt install gcc
```
- Download library 
```bash
git clone https://github.com/UCTRONICS/U6143_ssd1306.git
```
- Compile the source code 
```bash
cd U6143_SSD1306/C 
```
```bash
make 
```
## Run the display
```bash 
sudo ./display 
```

## python code


## Run
```bash
cd U6143_SSD1306/Ubuntu/python
sudo python3 ssd1306.py
```

## Use the rc.local file in Ubuntu

- Open the /lib/systemd/system/rc-local.service file and add the following content at the end of the file.
```
[Install]
WantedBy=multi-user.target
Alias=rc-local.service
```

- Create the rc.local file in the etc folder
```
sudo nano /etc/rc.local
```
- Add command to the rc.local file
```bash
cd /home/(User name)/U6143_SSD1306/C 
sudo make clean 
sudo make 
sudo ./display &
```
- Then add permissions to rc-local
```
sudo chmod +x /etc/rc.local
```
- Create a soft link in the /etc/systemd/system directory
```
sudo ln -s /lib/systemd/system/rc.local.service /etc/systemd/system/
```

- reboot your system

