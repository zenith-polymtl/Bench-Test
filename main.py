import tkinter as tk
from gui import BenchTestGUI
from serial_reader import SerialReader

if __name__ == "__main__":
    root = tk.Tk()
    serial_reader = SerialReader()
    app = BenchTestGUI(root, serial_reader)
    root.mainloop()
