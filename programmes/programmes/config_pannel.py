# -*- coding: utf-8 -*-
"""
Created on Wed Sep 28 14:35:29 2022
    Script to run to config Pannel
@author: louis
"""
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from tkinter import filedialog
from tkinter import Scrollbar
import re
import pathlib
import os
import sys
import json
import threading
import time

# DEFINE
JSON_ARGS = 'pannel_config_args'
END = 'end'
LEFT = "left"
RIGHT = "right"

# root_config window
root_config = tk.Tk()
root_config.geometry("370x300")
root_config.resizable(False, False)
root_config.title('Pannel configuration')
try:
    base_path = sys._MEIPASS
    logo_path = os.path.join(base_path, 'logo.ico')
    root_config.wm_iconbitmap(logo_path)
except:
    root_config.wm_iconbitmap('logo.ico')
# Data to store
username = tk.StringVar()
password = tk.StringVar()
broker_ipadr = tk.StringVar()
client_ipadr = tk.StringVar()
broker_port = tk.StringVar()
topic_vars = []
topic_label = []
topic_entry = []
OptionList = []
kill_threads = 0


def check_com_port():
    while(1):
        global kill_threads, texte, OptionList
        if(kill_threads):
            break
        OptionList.clear()
        comlist = serial.tools.list_ports.comports()
        for element in comlist:
            if((element.pid == 29987) & (element.vid == 6790) & (element.manufacturer == 'wch.cn')):
                OptionList.append(element.name)
        com_port_cb['values'] = OptionList
        time.sleep(1)


def update_topic_entries():
    num = int(sp.get())
    if(num != len(topic_entry)):
        config_button.forget()
        path_button.forget()
        read_button.forget()
        com_port_cb.forget()
        if(num < len(topic_entry)):
            for i in range(len(topic_entry)-num):
                topic_entry[len(topic_entry)-1].forget()
                topic_entry.pop(len(topic_entry)-1)
                topic_label[len(topic_label)-1].forget()
                topic_label.pop(len(topic_label)-1)
                topic_vars.pop(len(topic_vars)-1)
        else:
            for i in range(num-len(topic_entry)):
                # topics
                topic_vars.append(tk.StringVar())
                topic_label.append(ttk.Label(interface_config,
                                             text="Topic " + str(len(topic_label)) + " :",
                                             width=10))
                topic_label[len(topic_label)-1].pack(fill='x', expand=True)
                topic_entry.append(ttk.Entry(interface_config,
                                             textvariable=topic_vars[len(topic_vars)-1],
                                             width=20))
                topic_entry[len(topic_entry)-1].pack(fill='x', expand=True)
        config_button.pack(side=LEFT, fill='x', expand=True, pady=10)
        path_button.pack(side=LEFT, fill='x', expand=True, pady=10)
        com_port_cb.pack(side=RIGHT, fill='x', expand=True, pady=10)
        read_button.pack(side=LEFT, fill='x', expand=True, pady=10)
        canvas.update_idletasks()
        canvas.configure(scrollregion=canvas.bbox('all'),
                         yscrollcommand=scroll_y.set)


def int_to_2bytes(val: int):
    retHex1 = 0
    retByte1 = 0
    retByte2 = 0
    if(val > pow(16, 3)):
        retHex1 = val//pow(16, 3)
        val = val-retHex1*pow(16, 3)
        retHex2 = val//pow(16, 2)
    else:
        retHex2 = val//pow(16, 2)
    retByte1 = retHex1*16+retHex2
    retByte2 = val-retHex2*pow(16, 2)
    return retByte1, retByte2


def write_values_to_entries(jsondic: dict, arg: str):
    username_entry.delete(0, END)
    password_entry.delete(0, END)
    broker_ipadr_entry.delete(0, END)
    client_ipadr_entry.delete(0, END)
    broker_port_entry.delete(0, END)
    username_entry.insert(0, jsondic[arg].get('MQTT_username'))
    password_entry.insert(0, jsondic[arg].get('password'))
    broker_ipadr_entry.insert(0, jsondic[arg].get('broker_ip'))
    client_ipadr_entry.insert(0, jsondic[arg].get('client_ip'))
    broker_port_entry.insert(0, jsondic[arg].get('broker_port'))
    for i in range(len(topic_entry)):
        topic_entry[i].forget()
        topic_label[i].forget()
    topic_entry.clear()
    topic_label.clear()
    number_of_topic = 0
    while(('topic'+str(number_of_topic)) in jsondic[arg]):
        number_of_topic = number_of_topic+1
    if(number_of_topic != len(topic_entry)):
        topic_vars.clear()
        config_button.forget()
        path_button.forget()
        read_button.forget()
        com_port_cb.forget()
    for i in range(number_of_topic):
        topic_vars.append(tk.StringVar())
        topic_label.append(
            ttk.Label(interface_config, text="Topic " + str(i) + " :", width=10))
        topic_label[i].pack(fill='x', expand=True)
        topic_entry.append(
            ttk.Entry(interface_config, textvariable=topic_vars[i], width=20))
        topic_entry[i].pack(fill='x', expand=True)
        topic_entry[i].delete(0, END)
        topic_entry[i].insert(0, jsondic[arg].get(('topic'+str(i))))
    sp.config(state="normal")
    sp.delete(0, END)
    sp.insert(0, number_of_topic)
    sp.config(state="readonly")
    config_button.pack(side=LEFT, fill='x', expand=True, pady=10)
    path_button.pack(side=LEFT, fill='x', expand=True, pady=10)
    com_port_cb.pack(side=RIGHT, fill='x', expand=True, pady=10)
    read_button.pack(side=LEFT, fill='x', expand=True, pady=10)
    canvas.update_idletasks()
    canvas.configure(scrollregion=canvas.bbox('all'),
                     yscrollcommand=scroll_y.set)


def fill_config_entry(filepath: str):
    fpath = pathlib.Path(filepath)
    if((fpath.suffix == '.json')):
        json_file = open(fpath, "r")
        jsondic = json.loads(json_file.read())
        Avaiable_config = re.findall(JSON_ARGS+r'[1-9]{1,2}', str(jsondic))
        if(len(Avaiable_config) >= 2):
            select_config(jsondic)
        else:
            write_values_to_entries(jsondic, JSON_ARGS+'1')
            messagebox.showinfo("Config", "Configuration is loaded")
    else:
        messagebox.showerror("Config", "Configuration file isn't a json file")


def start_transfert_config():
    usr = username.get()
    pswd = password.get()
    bk_ip = broker_ipadr.get()
    clt_ip = client_ipadr.get()
    port = broker_port.get()
    topic_names = []
    for i in topic_vars:
        topic_names.append(i.get())
    if (com_port_cb.get() == ""):
        messagebox.showerror("ERROR", "COM PORT isn't detected")
    else:
        if((usr == '') | (pswd == '') | (clt_ip == '') | (bk_ip == '') | (port == '') | ('' in topic_names)):
            messagebox.showerror(
                "ERROR", "Current configuration have atleast one blank entry")
        else:
            Com_port_write(usr, pswd, bk_ip, clt_ip, port, topic_names)


def open_window_file_path():

    filepath = filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File",
                                          filetypes=(("Json files", "*.json"),
                                                     ("All files", "*.*")))
    if(filepath != ''):
        fill_config_entry(filepath)


def Com_port_read():
    if(com_port_cb.get() != ""):
        ser = serial.Serial(port=com_port_cb.get(), baudrate=115200, timeout=1)
        ser.close()
        ser.open()
        ser.write(chr(1).encode('latin_1') + chr(0).encode('latin_1')+chr(0).encode('latin_1'))
        esp_config = ser.read(1024)
        esp_config = esp_config.decode('latin_1')
        if(esp_config != ''):
            esp_config = esp_config[0:len(esp_config)]
            current_config = open('current_config.json', 'w')
            jsondic = json.loads(esp_config)
            json.dump(jsondic, current_config, indent=4)
            write_values_to_entries(jsondic)
            messagebox.showinfo("Config", "Read config done")
        else:
            messagebox.showerror("Config", "Configuration empty")
        ser.close()
    else:
        messagebox.showerror("ERROR", "COM PORT isn't detected")


def Com_port_write(usr: str, pswd: str, bk_ip: str, clt_ip: str, port: str, topic_names: list):
    ser = serial.Serial(port=com_port_cb.get(), baudrate=115200, timeout=1)
    ser.close()
    ser.open()
    dic_to_write = {JSON_ARGS+'1': {'MQTT_username': usr,
                                    'password': pswd,
                                    'broker_ip': bk_ip,
                                    'client_ip': clt_ip,
                                    'broker_port': port}}
    for i in range(len(topic_names)):
        dic_to_write[JSON_ARGS+'1']['topic'+str(i)] = topic_names[i]
    data_to_write = json.dumps(dic_to_write, indent=4,
                               separators=(',', ': '),
                               ensure_ascii=True)
    payloadsize = len(data_to_write)+1
    if(payloadsize > 255):
        [p1, p2] = int_to_2bytes(payloadsize)
        globalstr = chr(0).encode('latin_1')+chr(p1).encode('latin_1') + \
            chr(p2).encode('latin_1')+data_to_write.encode('latin_1')+'\0'.encode('latin1')
    else:
        globalstr = chr(0).encode('latin_1')+chr(0).encode('latin_1') + \
            chr(payloadsize).encode('latin_1') + \
            data_to_write.encode('latin_1')+'\0'.encode('latin1')
    ser.write(globalstr)
    ser.close()
    messagebox.showinfo("Config", "Write config done")


def select_config(json_dict: dict):
    root_config_select = tk.Toplevel(root_config)
    root_config_select.geometry("300x50")
    root_config_select.resizable(False, False)
    root_config_select.title('Choose your configuration')
    try:
        base_path = sys._MEIPASS
        logo_path = os.path.join(base_path, 'logo.ico')
        root_config_select.wm_iconbitmap(logo_path)
    except:
        root_config_select.wm_iconbitmap('logo.ico')
    # frame
    interface_select = ttk.Frame(root_config_select)
    interface_select.pack(padx=10, pady=10, fill='x', expand=False)
    selection = ttk.Label(interface_select, text="Configuration:")
    selection.pack(side="left", fill='x', expand=False)
    # combo box to select com port
    config = tk.StringVar()
    config_cb = ttk.Combobox(interface_select, textvariable=config, width=20, state="readonly")
    config_cb.pack(side=LEFT)
    config_cb['values'] = list(json_dict.keys())
    # Ok button
    confirm_button = ttk.Button(interface_select, text="Confirm",
                                command=lambda: confirm_config(config.get(), root_config_select,
                                                               json_dict))
    confirm_button.pack(side="right")
    root_config_select.mainloop()


def confirm_config(config: str, root_config_select: tk.Toplevel, json_dict: dict):
    if(config != ''):
        write_values_to_entries(json_dict, config)
        messagebox.showinfo("Config", "Configuration is loaded")
    else:
        messagebox.showerror("Config", "Blank configuration")
    root_config_select.destroy()


# frame
canvas = tk.Canvas(root_config, bd=0, highlightthickness=0)
scroll_y = tk.Scrollbar(root_config, orient="vertical", command=canvas.yview)
scroll_y.pack(fill='y', side='right')
interface_config = ttk.Frame(canvas)
# interface_config.pack(padx=10, pady=10, fill='x', expand=False)


# MQTT username
MQTT_username_label = ttk.Label(interface_config, text="MQTT_username:", width=10)
MQTT_username_label.pack(fill='x', expand=True)
username_entry = ttk.Entry(interface_config, textvariable=username, width=20)
username_entry.pack(fill='x', expand=True)
username_entry.focus()

# MQTT password
MQTT_password_label = ttk.Label(interface_config, text="Password:", width=10)
MQTT_password_label.pack(fill='x', expand=True)

password_entry = ttk.Entry(interface_config, textvariable=password, show="*", width=20)
password_entry.pack(fill='x', expand=True)

# Broker IP adress
ip_label = ttk.Label(interface_config, text="Broker ip adress:", width=10)
ip_label.pack(fill='x', expand=True)

broker_ipadr_entry = ttk.Entry(interface_config, textvariable=broker_ipadr, width=20)
broker_ipadr_entry.pack(fill='x', expand=True)

# Client IP adress
ip_label = ttk.Label(interface_config, text="Client ip adress:", width=10)
ip_label.pack(fill='x', expand=True)

client_ipadr_entry = ttk.Entry(interface_config, textvariable=client_ipadr, width=20)
client_ipadr_entry.pack(fill='x', expand=True)

# Broker port
broker_port_label = ttk.Label(interface_config, text="Broker port:", width=10)
broker_port_label.pack(fill='x', expand=True)

broker_port_entry = ttk.Entry(interface_config, textvariable=broker_port, width=20)
broker_port_entry.pack(fill='x', expand=True)

# spinbox for the number of topics
spinbox_label = ttk.Label(interface_config, text="Number of topics:", width=10)
spinbox_label.pack(fill='x', expand=True)

sp = tk.Spinbox(interface_config, from_=0, to=20, width=20,
                command=update_topic_entries, state="readonly")
sp.pack(side="top")

# combo box to select com port
com_port = tk.StringVar()
com_port_cb = ttk.Combobox(interface_config, textvariable=com_port, width=8, state="readonly")
com_port_cb.pack(side=RIGHT)

# Config button
config_button = ttk.Button(
    interface_config, text="Configuration", command=start_transfert_config)
config_button.pack(side=LEFT, fill='x', expand=True, pady=10)


# Path button
path_button = ttk.Button(interface_config, text="...", command=open_window_file_path, width=10)
path_button.pack(side=LEFT, pady=10)

# Read button
read_button = ttk.Button(interface_config, text="Read Config", command=Com_port_read, width=15)
read_button.pack(side=RIGHT, pady=10)
canvas.create_window(0, 0, anchor='nw', window=interface_config)
canvas.update_idletasks()
canvas.configure(scrollregion=canvas.bbox('all'),
                 yscrollcommand=scroll_y.set)
canvas.pack(side='left', padx=10, pady=10)


com_thread = threading.Thread(target=check_com_port, daemon=True)
com_thread.start()
# main loop for tk window
root_config.mainloop()
kill_threads = 1
