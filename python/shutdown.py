import os
import signal
import subprocess
from board import SCL, SDA
import busio
import adafruit_ssd1306


def main():
    pid = int((subprocess.check_output(
        "ps aux | grep python3 | grep -m 1 chart_performance.py | awk '{printf \"%s\", $2}'",
        shell=True)
              .decode("utf-8")))

    try:
        os.kill(pid, signal.SIGTERM)
        print("Blanked out OLED screen to avoid burn-in. Shutdown signal sent to the background app.")

        i2c = busio.I2C(SCL, SDA)
        disp = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c)
        disp.fill(0)
        disp.show()

    except ProcessLookupError:
        print("Invalid PID or process not found.")


if __name__ == "__main__":
    main()
