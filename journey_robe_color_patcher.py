import tkinter as tk
from tkinter import messagebox, ttk 
from PIL import Image, ImageTk
import struct
import winreg

class JourneyColorPatcher:
    def __init__(self, master):
        self.master = master
        self.master.title("Journey robe color patcher")
        self.master.geometry("400x60")
        
        self.offsets = [0x161D7F, 0x2169F7]
        self.values = [1, 2, 3] 
        self.binary_file_path = self.get_binary_file_path()  
        
        self.load_thumbnails()
        self.create_buttons()
        self.create_separator()
        self.create_current_label()

    def load_thumbnails(self):
        self.thumbnails = []
        for i in self.values:
            thumbnail_path = f"tier_{i + 1}.webp" 
            thumbnail_img = ImageTk.PhotoImage(Image.open(thumbnail_path).resize((35, 35)))  
            self.thumbnails.append(thumbnail_img)

    def create_buttons(self):
        for value, thumbnail in zip(self.values, self.thumbnails):
            frame = tk.Frame(self.master)
            frame.pack(side=tk.LEFT, padx=5)
            
            img_label = tk.Label(frame, image=thumbnail)
            img_label.pack(side=tk.LEFT, padx=5)
            
            button = tk.Button(frame, text=f"Tier {value + 1}", 
                               command=lambda val=value: self.confirm_and_write(val), 
                               width=5, height=2)
            button.pack(side=tk.TOP)

    def create_separator(self):
        separator = ttk.Separator(self.master, orient='vertical')
        separator.pack(side=tk.LEFT, padx=5, fill='y')

    def create_current_label(self):
        self.current_label = tk.Label(self.master, text="Current")
        self.current_label.pack(side=tk.LEFT, padx=15)
        self.update_current_value_label(self.offsets[0])

    def write_to_file(self, offsets, value):
        data = struct.pack('<I', value)
        with open(self.binary_file_path, "r+b") as file:
            for offset in offsets:
                file.seek(offset)
                file.write(data)

    def read_value_at_offset(self, offset):
        with open(self.binary_file_path, "rb") as file:
            file.seek(offset)
            value = struct.unpack('<I', file.read(4))[0]
        return value

    def confirm_and_write(self, value):
        confirmation = messagebox.askyesno("Confirmation", f"Do you want to set Tier {value + 1}?")
        if confirmation:
            self.write_to_file(self.offsets, value)
            self.update_current_value_label(self.offsets[0])

    def update_current_value_label(self, offset):
        with open(self.binary_file_path, "rb") as file:
            file.seek(offset)
            value = struct.unpack('<I', file.read(4))[0]

        self.current_label.config(image=self.thumbnails[value - 1])

    def get_binary_file_path(self):
        steam_path = self.get_steam_installation_path()
        if steam_path:
            return f"{steam_path}\\SteamApps\\common\\Journey\\Journey.exe"
        else:
            raise ValueError("Unable to find Steam installation path.")

    def get_steam_installation_path(self):
        try:
            key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, r"SOFTWARE\Valve\Steam")
            value, _ = winreg.QueryValueEx(key, "SteamPath")
            winreg.CloseKey(key)
            return value
        except FileNotFoundError:
            messagebox.showerror("Error", "Steam not found in registry.")
            return None
        except Exception as e:
            messagebox.showerror("Error", f"Error accessing registry: {e}")
            return None


def main():
    root = tk.Tk()
    app = JourneyColorPatcher(root)
    root.mainloop()

if __name__ == "__main__":
    main()
