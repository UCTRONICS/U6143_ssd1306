from board import SCL, SDA
from PIL import Image, ImageDraw, ImageFont
from enum import Enum, auto
from datetime import datetime
import busio
import adafruit_ssd1306
import numpy as np
import time
import subprocess
import signal


class Settings:
    screen_bottom: int
    screen_right: int
    screen_left: int
    screen_top: int

    seconds_per_page: int
    refresh_data_interval: int

    chart_top: int
    chart_bottom: int

    text_height: int
    chart_max_values: int

    draw: ImageDraw
    font: ImageFont
    disp: adafruit_ssd1306
    image: Image

    def __init__(self):
        # Time Jump to the next screen (or page) every 3 seconds
        self.seconds_per_page = 3

        # Retrieve performance data from the system every 2 seconds
        self.refresh_data_interval = 2

        # Create the I2C interface, the SSD1406 OLED class, the image and untimely the draw object
        i2c = busio.I2C(SCL, SDA)
        self.disp = adafruit_ssd1306.SSD1306_I2C(128, 32, i2c)
        self.disp.fill(0)
        self.disp.show()

        width = self.disp.width
        height = self.disp.height
        self.screen_top = 0
        self.screen_bottom = height - 1
        self.screen_left = 0
        self.screen_right = width - 1

        self.image = Image.new("1", (width, height))

        # Setup the drawing and font objects
        self.draw = ImageDraw.Draw(self.image)
        self.font = ImageFont.load_default()

        self.text_height = self.draw.textsize("MMMM", self.font)[1]
        self.chart_top = self.text_height + 2
        self.chart_bottom = self.screen_bottom
        self.chart_max_values = int(
            (width - 4) / 2)  # the 4 is for the margins, and it takes 2 pixels for each chart value


class ChartType(Enum):
    LINE = auto()
    BAR = auto()


class MetricType(Enum):
    IP_ADDR = auto()
    HOSTNAME = auto()
    DISK = auto()
    CPU = auto()
    CPU_TEMP = auto()
    MEMORY = auto()


class Metric:
    shell: str
    fmt: str
    active: bool
    last_value: str
    chartable: bool
    chart_data: []

    def __init__(self, shell, fmt, chartable=True):
        self.shell = shell
        self.fmt = fmt
        self.active = False
        self.last_value = ""
        self.chartable = chartable
        self.chart_data = np.empty(shape=(0,), dtype=float)

    def text(self):
        split = (self.last_value + ",,,,,").split(',')
        return self.fmt.format(split[0], split[1], split[2], split[3])

    def validate(self):
        return


class Page:
    name: str
    page_metrics: []

    def __init__(self, name, metric_types):
        self.name = name
        self.page_metrics = np.empty(shape=(0,), dtype=MetricType)

        for page_metric in metric_types:
            try:
                m = metrics[page_metric]
                m.active = True
                self.page_metrics = np.append(self.page_metrics, page_metric)
            except KeyError as e:
                raise ValueError("metric \"{0}\n is not defined, review setup_metrics() method for value metric names")

    def display(self):
        settings.draw.rectangle((0, 0, settings.screen_right, settings.screen_bottom), outline=0, fill=0)

        row = 0
        for metric_type in self.page_metrics:
            metric = metrics[metric_type]
            settings.draw.text((settings.screen_left, row), metric.text(), font=settings.font, fill=255)
            row += settings.text_height

    def validate(self):
        if len(self.page_metrics) == 0:
            raise ValueError("\"{0}\" page must have at least 1 performance metric"
                             .format(self.name))


class TextPage(Page):

    def display(self):
        super().display()

        settings.disp.image(settings.image)
        settings.disp.show()

    def validate(self):
        super().validate()
        if len(self.page_metrics) > 3:
            raise ValueError("\"{0}\" text page must have between 1-3 performance metrics. Try breaking them up into "
                             "multiple pages"
                             .format(self.name))


class ChartPage(Page):
    chart_type: ChartType
    chart_high: int
    chart_low: int

    def __init__(self, name, metric_types, chart_type, chart_high=-1, chart_low=-1):
        super().__init__(name, metric_types)
        self.chart_type = chart_type
        self.chart_high = chart_high
        self.chart_low = chart_low

    def display(self):
        super().display()

        metric = metrics[self.page_metrics[0]]
        value_min = np.min(metric.chart_data)  # if metric.chart_low == -1 else metric.chart_low
        value_max = np.max(metric.chart_data)  # if metric.chart_high == -1 else metric.chart_high
        value_max += (value_max - value_min) * 0.1

        settings.draw.rectangle((settings.screen_left, settings.chart_top, settings.screen_right, settings.screen_bottom), outline=1, fill=0)

        if self.chart_type == ChartType.LINE:
            last_row = -1
            col = settings.screen_right - 2
            for value in metric.chart_data:
                row = settings.chart_top - (value - value_min) * (settings.chart_top - settings.chart_bottom) / (
                        value_max - value_min)
                settings.draw.line([col, row, col + 2, row if last_row == -1 else last_row], width=1, fill=1)
                last_row = row
                col -= 2

        if self.chart_type == ChartType.BAR:
            col = settings.screen_right
            for value in metric.chart_data:
                row = (settings.chart_top - (value - value_min) *
                       (settings.chart_top - settings.chart_bottom) /
                       (value_max - value_min))
                settings.draw.line([col, row, col, settings.screen_bottom], width=1, fill=1)
                col -= 2

        settings.disp.image(settings.image)
        settings.disp.show()

    def validate(self):
        super().validate()
        if len(self.page_metrics) > 1:
            raise ValueError("\"{0}\" is a chart page, which cannot contain more than 1 performance metric"
                             .format(self.name))

        if not metrics[self.page_metrics[0]].chartable:
            raise ValueError("\"{0}\" performance metric must be chartable".format(self.name))


class MeterPage(Page):
    warning_value: int
    meter_high: int
    meter_low: int
    num_meter_boxes: int

    def __init__(self, name, metric_types, meter_high, meter_low, num_meter_boxes=10, warning_value=0):
        super().__init__(name, metric_types)
        self.meter_high = meter_high
        self.meter_low = meter_low
        self.num_meter_boxes = num_meter_boxes
        self.warning_value = warning_value

    def display(self):
        super().display()

        last_value = int(float(metrics[self.page_metrics[0]].last_value.split(",")[0]))
        settings.draw.rectangle((settings.screen_left, settings.chart_top, settings.screen_right, settings.screen_bottom), outline=1, fill=0)

        meter_left = 3
        meter_right = settings.screen_right - 3
        meter_top = settings.chart_top + 3
        meter_bottom = settings.screen_bottom - 3

        box_width_padding = (meter_right - meter_left) / self.num_meter_boxes
        box_width = box_width_padding - 3

        if self.warning_value != 0:
            self.draw_warning()

        value_line = ((last_value - self.meter_low) /
                      (self.meter_high - self.meter_low) *
                      (settings.screen_right - 1))

        self.draw_meter_boxes(box_width, box_width_padding, meter_bottom, meter_left, meter_top, value_line)

        settings.disp.image(settings.image)
        settings.disp.show()

    def draw_meter_boxes(self, box_width, box_width_padding, meter_bottom, meter_left, meter_top, value_line):
        for i in range(self.num_meter_boxes):
            x = meter_left + i * box_width_padding
            settings.draw.rectangle((x, meter_top, x + box_width, meter_bottom),
                                    outline=1,
                                    fill=0 if value_line < x + box_width else 1)

    def draw_warning(self):
        warning_line = ((self.warning_value - self.meter_low) /
                        (self.meter_high - self.meter_low) *
                        (settings.screen_right - 5))
        settings.draw.line((warning_line, settings.chart_top, warning_line, settings.screen_bottom), width=1, fill=1)

        inc = 3
        x1 = warning_line + inc
        y1 = settings.chart_top
        x2 = warning_line
        y2 = settings.chart_top + inc
        while x1 < settings.screen_right * 2:
            settings.draw.line((x1, y1, x2, y2), width=1, fill=1)
            x1 += inc
            y2 += inc

    def validate(self):
        super().validate()

        if len(self.page_metrics) > 1:
            raise ValueError("\"{0}\" is a meter page, which requires 1 performance metric".format(self.name))

        if not metrics[self.page_metrics[0]].chartable:
            raise ValueError("\"{0}\" performance metric must be chartable".format(self.name))

        if self.meter_low >= self.meter_high:
            raise ValueError("\"{0}\" meter_low must be less than meter_high".format(self.name))


def display_splash():
    settings.draw.rectangle((0, 0, settings.screen_right, settings.screen_bottom), outline=0, fill=0)
    splash_font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20)

    settings.draw.text((0, 5), "UCTRONICS", font=splash_font, fill=255)
    settings.disp.image(settings.image)
    settings.disp.show()


def refresh_data():
    for metric in metrics.values():
        if not metric.active:
            continue

        metric.last_value = subprocess.check_output(metric.shell, shell=True).decode("utf-8")

        if metric.chartable:
            if len(metric.chart_data) >= settings.chart_max_values:
                metric.chart_data = np.delete(metric.chart_data, -1)

            metric.chart_data = np.insert(metric.chart_data, 0, metric.last_value.split(",")[0])


def setup_metrics():
    ret = {
        MetricType.IP_ADDR:
            Metric(
                shell="hostname -I | cut -d' ' -f1 | awk '{printf \"%s\", $1}'",
                fmt="IP Addr: {0}",
                chartable=False),
        MetricType.HOSTNAME:
            Metric(
                shell="hostname | cut -d' ' -f1 | awk '{printf \"%s\", $1}'",
                fmt="Hostname: {0}",
                chartable=False),
        MetricType.DISK:
            Metric(
                shell='df -h | awk \'$NF=="/"{printf "%d,%d,%s",$3,$2,$5}\'',
                fmt="Disk: {0}/{1}GB ({2})"),
        MetricType.CPU:
            Metric(
                shell="top -bn1 | grep load | awk '{printf \"%.2f\", $(NF-2)}'",
                fmt="CPU Util: {0}%"),
        MetricType.CPU_TEMP:
            Metric(
                shell="cat /sys/class/thermal/thermal_zone0/temp | awk {'printf \"%.2f\", $1/1000*1.8+32 }'",
                fmt="CPU temp: {0}°"),
        MetricType.MEMORY:
            Metric(
                shell="free -m | awk 'NR==2{printf \"%s,%s,%.2f\", $3,$2,$3*100/$2 }'",
                fmt="Mem: {0}/{1}MB ({2}%)")
    }
    return ret


def define_pages():
    ret = [
        TextPage(name="summary",  # This data on this page doesn't "chart" so well...
                 metric_types=[MetricType.IP_ADDR, MetricType.HOSTNAME, MetricType.DISK]),

        TextPage(name="summary2",  # This page is here primarily as an example
                 metric_types=[MetricType.CPU, MetricType.CPU_TEMP, MetricType.MEMORY]),

        MeterPage(name="TempMeter",
                  meter_low=0,
                  meter_high=200,
                  warning_value=185,
                  metric_types=[MetricType.CPU_TEMP]),

        MeterPage(name="DiskMeter",
                  meter_low=0,
                  meter_high=118,
                  num_meter_boxes=20,
                  metric_types=[MetricType.DISK]),

        ChartPage(name="CPU",
                  chart_type=ChartType.BAR,
                  metric_types=[MetricType.CPU],
                  chart_low=0,
                  chart_high=100),

        ChartPage(name="Temp",
                  chart_type=ChartType.LINE,
                  metric_types=[MetricType.CPU_TEMP]),

        ChartPage(name="Memory",
                  chart_type=ChartType.BAR,
                  metric_types=[MetricType.MEMORY])
    ]

    for pg in ret:
        pg.validate()

    return ret


def shutdown_handler(signum, frame):
    print("Received shutdown signal")
    raise SystemExit("Shutting down")


def main():
    signal.signal(signal.SIGINT, shutdown_handler)
    signal.signal(signal.SIGTERM, shutdown_handler)

    global settings
    settings = Settings()

    global metrics
    metrics = setup_metrics()

    global pages
    pages = define_pages()

    # Display and pause to allow the users to "bask in the beauty" of the splash screen
    display_splash()
    time.sleep(2)

    page_num = 0
    while True:
        seconds = datetime.now().second
        if seconds % settings.refresh_data_interval == 0:
            refresh_data()

        if seconds % settings.seconds_per_page == 0:
            pages[page_num].display()
            page_num = (page_num + 1) % len(pages)

        time.sleep(1)


# Define a few globals that are used throughout the script
settings = Settings
metrics = {}
pages = dict()

if __name__ == "__main__":
    main()
