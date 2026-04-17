import tkinter as tk
import asyncio
import threading
from bleak import BleakScanner
from bleak import BleakClient

# Set up window
root = tk.Tk()
root.title("ESP32 wifi Config")
root.geometry("400x400")

# Label
tk.Label(root, text = "Wifi SSID").pack()
ssid_entry = tk.Entry(root)
ssid_entry.pack()

tk.Label(root, text = "Password").pack()
pass_entry = tk.Entry(root, show = "*")
pass_entry.pack()

# Button
scan_btn = tk.Button(root, text = "Scan BLE")
scan_btn.pack(pady = 10)

connect_btn = tk.Button(root, text = "Connect")
connect_btn.pack(pady = 10)

send_btn = tk.Button(root, text = "Send Wifi")
send_btn.pack(pady = 10)

device_list = tk.Listbox(root)
device_list.pack(fill = "both", expand = "true")

# Scan func
async def scan_ble():
    devices = await BleakScanner.discover()
    device_list.delete(0, tk.END)
    for d in devices:
        device_list.insert(tk.END, f"{d.name} - {d.address}") 

def run_scan():
    asyncio.run(scan_ble())

def on_scan():
    threading.Thread(target = run_scan).start()

scan_btn.config(command = on_scan)

# Connect func
client = None

async def connect_device(address):
    global client
    client = BleakClient(address)
    await client.connect()
    print("Connected!")

def run_connect(address):
    asyncio.run(connect_device(address))

def on_connect():
    selected = device_list.get(tk.ACTIVE)
    address = selected.split(" - ")[1]
    threading.Thread(target = run_connect, args=(address,)).start()

connect_btn.config(command= on_connect)

# Send wifi func
WRITE_UUID = "00005678-0000-1000-8000-00805f9b34fb"

async def send_wifi():
    ssid = ssid_entry.get()
    password = pass_entry.get()

    data = f"{ssid},{password}".encode()

    await client.write_gatt_char(WRITE_UUID, data)
    print("Sent!")
    # services = client.services
    # for service in services:
    #     for char in service.characteristics:
    #         print(f"Đang có Characteristic: {char.uuid}")

def on_send():
    threading.Thread(target=lambda: asyncio.run(send_wifi())).start()

send_btn.config(command = on_send)

NOTIFY_UUID = "00009abc-0000-1000-8000-00805f9b34fb"

def notification_handler(sender, data):
    print("Received:", data.decode())

async def start_notify():
    await client.start_notify(NOTIFY_UUID, notification_handler)

async def connect_device(address):
    global client
    client = BleakClient(address)
    await client.connect()
    await client.start_notify(NOTIFY_UUID, notification_handler)

status_label = tk.Label(root, text = "Status: Idle")
status_label.pack()

def notification_handler(sender, data):
    msg = data.decode()
    status_label.config(text = f"Status: {msg}")

root.mainloop()