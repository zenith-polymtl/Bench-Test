import tkinter as tk
from tkinter import ttk, messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
from collections import defaultdict
from datetime import datetime
import csv
import os

class BenchTestGUI:
    def __init__(self, root, serial_reader):
        self.root = root
        self.root.title("Bench Test GUI")
        self.serial_reader = serial_reader

        self.setup_ui()
        self.setup_plot()
        self.update_serial_monitor()

    def setup_ui(self):
        # COM port
        ttk.Label(self.root, text="Select COM Port:").pack(pady=5)
        self.combobox = ttk.Combobox(self.root, values=self.serial_reader.list_serial_ports())
        self.combobox.pack(pady=5)
        ttk.Button(self.root, text="Connect", command=self.connect_serial).pack(pady=5)

        # LED indicators
        self.led_frame = ttk.Frame(self.root)
        self.led_frame.pack(pady=5)

        self.led_yellow = tk.Label(self.led_frame, text="🟡", font=("Arial", 24))
        self.led_yellow.grid(row=0, column=0, padx=10)

        self.led_red = tk.Label(self.led_frame, text="🔴", font=("Arial", 24))
        self.led_red.grid(row=0, column=1, padx=10)

        self.led_blue = tk.Label(self.led_frame, text="🔵", font=("Arial", 24))
        self.led_blue.grid(row=0, column=2, padx=10)

        # Control buttons
        ttk.Button(self.root, text="Start", command=self.start_acquisition).pack(pady=5)
        ttk.Button(self.root, text="Stop", command=self.stop_acquisition).pack(pady=5)

        # Serial monitor
        self.serial_text = tk.Text(self.root, height=10, width=80)
        self.serial_text.pack(pady=5)

    def setup_plot(self):
        self.fig, self.ax = plt.subplots(figsize=(6, 4))
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def connect_serial(self):
        port = self.combobox.get()
        try:
            self.serial_reader.connect(port)
            messagebox.showinfo("Success", f"Connected to {port}")
        except Exception as e:
            messagebox.showerror("Error", str(e))

    def start_acquisition(self):
        self.serial_reader.start()

    def stop_acquisition(self):
        self.serial_reader.stop()
        self.save_csv()
        self.plot_force_vs_pwm()

    def update_serial_monitor(self):
        try:
            if self.serial_reader.serial_port and self.serial_reader.serial_port.in_waiting:
                ...
        except Exception as e:
            print(f"[Serial check error]: {e}")
            try:
                line = self.serial_reader.serial_port.readline().decode('utf-8').strip()
                if line:
                    # Afficher dans le moniteur
                    self.serial_text.insert(tk.END, line + "\n")
                    self.serial_text.see(tk.END)

                    # Analyser les boutons
                    parts = line.split(',')
                    if len(parts) >= 3:
                        try:
                            sw1 = int(parts[0].strip())
                            red = int(parts[1].strip())
                            blue = int(parts[2].strip())

                            self.led_yellow.config(bg="yellow" if sw1 else "gray")
                            self.led_red.config(bg="red" if red else "gray")
                            self.led_blue.config(bg="blue" if blue else "gray")
                        except:
                            pass
            except Exception as e:
                print(f"[Serial read error]: {e}")

        self.root.after(100, self.update_serial_monitor)

    

    def save_csv(self):
        data = self.serial_reader.get_data()
        now = datetime.now().strftime("%Y%m%d_%H%M%S")

        # Créer dossier s'il n'existe pas
        output_folder = "data_csv"
        os.makedirs(output_folder, exist_ok=True)

        filename = os.path.join(output_folder, f"bench_data_{now}.csv")

        with open(filename, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerow(["timestamp", "percent", "pwm", "load_cell_raw", "force"])
            for i in range(len(data['time'])):
                writer.writerow([
                    data['time'][i],
                    data['percent'][i],
                    data['pwm'][i],
                    data['load_cell_raw'][i],
                    data['force'][i]
                ])

        messagebox.showinfo("Saved", f"Data saved to {filename}")
        
    def plot_force_vs_pwm(self):
        data = self.serial_reader.get_data()
        pwm_values = data['pwm']
        force_values = data['force']

        # Convert PWM 1000-2000 to 0-100%
        pwm_percent = [(pwm - 1000) / 10.0 for pwm in pwm_values]

        # Moyenne force par PWM%
        grouped = defaultdict(list)
        for pwm, force in zip(pwm_percent, force_values):
            key = round(pwm)
            grouped[key].append(force)

        avg_force = {k: sum(v) / len(v) for k, v in grouped.items()}
        sorted_keys = sorted(avg_force.keys())
        sorted_avg_force = [avg_force[k] for k in sorted_keys]

        self.ax.clear()
        self.ax.plot(sorted_keys, sorted_avg_force, marker='o')
        self.ax.set_xlabel("PWM (%)")
        self.ax.set_ylabel("Force (N)")
        self.ax.set_title("Force Moyenne vs PWM (%)")
        self.ax.grid(True)

        self.canvas.draw()
