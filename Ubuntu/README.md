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
cd U6143_ssd1306/C 
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
cd U6143_ssd1306/Ubuntu/python
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
#!/bin/sh
cd /home/(User name)/U6143_ssd1306/C
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
sudo ln -s /lib/systemd/system/rc-local.service /etc/systemd/system/
```

- reboot your system

- Check that systemd's rc-local.service started up correctly
```
sudo systemctl status rc-local.service

● rc-local.service - /etc/rc.local Compatibility
     Loaded: loaded (/lib/systemd/system/rc-local.service; enabled-runtime; vendor preset: enabled)
    Drop-In: /usr/lib/systemd/system/rc-local.service.d
             └─debian.conf
     Active: active (running) since Fri 2021-09-24 16:51:54 UTC; 2min 15s ago
       Docs: man:systemd-rc-local-generator(8)
    Process: 1695 ExecStart=/etc/rc.local start (code=exited, status=0/SUCCESS)
      Tasks: 6 (limit: 9257)
     Memory: 28.6M
     CGroup: /system.slice/rc-local.service
             ├─1903 sudo ./display
             ├─1907 ./display
             ├─3601 sh -c top -bn1 | grep load | awk '{printf "%.2f", $(NF-2)}'
             ├─3602 top -bn1
             ├─3603 grep load
             └─3604 awk {printf "%.2f", $(NF-2)}

Sep 24 16:51:49 pi-1 rc.local[1736]: cc    -c -o display.o display.c
Sep 24 16:51:52 pi-1 rc.local[1736]: cc    -c -o ssd1306_i2c.o ssd1306_i2c.c
Sep 24 16:51:54 pi-1 rc.local[1736]: gcc display.o ssd1306_i2c.o -o display -g -rdynamic
Sep 24 16:51:54 pi-1 sudo[1735]: pam_unix(sudo:session): session closed for user root
Sep 24 16:51:54 pi-1 systemd[1]: Started /etc/rc.local Compatibility.
```
