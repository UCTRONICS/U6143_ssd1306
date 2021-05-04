#/bin/sh

#sudo mkdir -p /usr/local/bin && sudo cp ./display /usr/local/bin/

tar -Pcvf U6143_ssd1306.tar \
	/etc/systemd/system/U6143_ssd1306.service \
	/usr/local/bin/display \
	/usr/local/bin/gpio \
	/usr/local/lib/libwiringPiDev.so \
	/usr/local/lib/libwiringPiDev.so.2.60 \
	/usr/local/lib/libwiringPi.so \
	/usr/local/lib/libwiringPi.so.2.60

## To extract:
# sudo tar -xvhf U6143_ssd1306.tar
# sudo systemctl daemon-reload
# sudo systemctl enable U6143_ssd1306.service
