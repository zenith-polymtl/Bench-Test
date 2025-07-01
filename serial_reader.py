import serial
import serial.tools.list_ports
import threading
import re
from datetime import datetime

class SerialReader:
    def __init__(self):
        self.serial_port = None
        self.running = False

        self.timestamps = []
        self.percent_values = []
        self.pwm_values = []
        self.raw_load_cell_values = []
        self.force_values = []

        # Regex patterns
        self.pattern = re.compile(
            r'Percent:\s*([\d.]+)\s*,\s*Pwm:\s*([\d.]+)\s*,\s*Load\s+cell\s+raw\s+:\s*([-\d.]+)\s*,\s*Force\s+:\s*([-\d.]+)'
        )

        # Optional: create file with timestamp
        self.filename = f"bench_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        # Uncomment below to initialize CSV header
        # with open(self.filename, 'w') as f:
        #     f.write("timestamp,percent,pwm,load_cell_raw,force\n")

    def list_serial_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def connect(self, port):
        self.serial_port = serial.Serial(port, 115200, timeout=1)

    def start(self):
        if self.serial_port and self.serial_port.is_open:
            self.running = True
            threading.Thread(target=self.read_serial, daemon=True).start()

    def stop(self):
        self.running = False

    def read_serial(self):
        while self.running:
            try:
                line = self.serial_port.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print("[SERIAL]", line)
                    match = self.pattern.search(line)
                    if match:
                        percent = float(match.group(1))
                        pwm = float(match.group(2))
                        raw = float(match.group(3))
                        force = float(match.group(4))
                        timestamp = datetime.now()

                        self.timestamps.append(timestamp)
                        self.percent_values.append(percent)
                        self.pwm_values.append(pwm)
                        self.raw_load_cell_values.append(raw)
                        self.force_values.append(force)

            except Exception as e:
                print(f"[ERROR] Serial read failed: {e}")

    def get_data(self):
        return {
            'time': self.timestamps,
            'percent': self.percent_values,
            'pwm': self.pwm_values,
            'load_cell_raw': self.raw_load_cell_values,
            'force': self.force_values
        }
