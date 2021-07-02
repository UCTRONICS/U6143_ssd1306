#/bin/sh

#sudo mkdir -p /usr/local/bin && sudo cp ./display /usr/local/bin/

tar -Pcvf U6143_ssd1306.tar \
	/etc/systemd/system/U6143_ssd1306.service \
	/usr/local/bin/display \

## To extract:
# sudo tar -xvhf U6143_ssd1306.tar
# sudo systemctl daemon-reload
# sudo systemctl enable U6143_ssd1306.service
