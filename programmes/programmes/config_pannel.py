# -*- coding: utf-8 -*-
"""
Created on Wed Sep 28 14:35:29 2022
    Script to run to config Pannel
@author: louis
"""
import serial.tools.list_ports
from tkinter import *
import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import pathlib
import os
import json

# root_config window
root_config = tk.Tk()
root_config.geometry("500x300")
root_config.resizable(False, False)
root_config.title('Pannel configuration')

# Data to store
username = tk.StringVar()
password = tk.StringVar()
ipadr = tk.StringVar()
broker_port = tk.StringVar()
topic = tk.StringVar()

fileObject = open("formatJSON.json", "r")
jsonContent = fileObject.read()
obj_python = json.loads(jsonContent)
print(obj_python)
print(obj_python['pannel_config_args'])
print(type(obj_python))


def Ok_clicked(root_to_close,filepath):
    fpath=pathlib.Path(filepath.get())
    print(fpath)
    found_file=os.path.exists(fpath)
    if(found_file&(fpath.suffix=='.json')):
        config_file = open(fpath, "r")
        print(config_file.read())
        obj_python = config_file.read()
        json_file = json.loads(obj_python)
        print(json_file)
        messagebox.showinfo("Config", "Configuration is loaded")
    else:
        if((fpath.suffix!='.json')&(found_file==0)):
            messagebox.showerror("Config", "Configuration file isn't a json file\nConfiguration isn't loaded file not found")
        else:
            if(found_file==0):
                messagebox.showerror("Config", "Configuration isn't loaded file not found")
            else:
                messagebox.showerror("Config", "Configuration file isn't a json file")
    root_to_close.destroy()
    

def Configuration_clicked():
    """ callback when the config button clicked
    """
    usr = username.get()
    pswd = password.get()
    pannel_ip = ipadr.get()
    port =  broker_port.get()
    MQQT_topic = topic.get();
    print(usr,pswd,pannel_ip,port,MQQT_topic)
    
    
def File_path_clicked():
    
    root_config_fpath = tk.Toplevel(root_config)
    root_config_fpath.geometry("500x50")
    root_config_fpath.resizable(False, False)
    root_config_fpath.title('Path file')
    filepath = tk.StringVar()
    # frame
    interface_path = ttk.Frame(root_config_fpath)
    interface_path.pack(padx=10, pady=10, fill='x', expand=False)

    # Path 
    path = ttk.Label(interface_path, text="Path:")
    path.pack(side = LEFT,fill='x', expand=False)
    
    file = ttk.Entry(interface_path, textvariable=filepath)
    file.pack(side = LEFT,fill='x', expand=True)
    
    # Ok button
    config_button = ttk.Button(interface_path, text="OK", command=lambda:Ok_clicked(root_config_fpath,filepath))
    config_button.pack(side = RIGHT)
    
    root_config_fpath.mainloop()
    
    

# frame
interface_config = ttk.Frame(root_config)
interface_config.pack(padx=10, pady=10, fill='x', expand=False)


# Config part
Config = ttk.Label(interface_config, text="Config:")
Config.pack(fill='x', expand=False)

# MQTT username
MQTT_Username = ttk.Label(interface_config, text="MQTT_Username:")
MQTT_Username.pack(fill='x', expand=True)

username_entry = ttk.Entry(interface_config, textvariable=username)
username_entry.pack(fill='x', expand=True)
username_entry.focus() 

# MQTT password
MQTT_password = ttk.Label(interface_config, text="Password:")
MQTT_password.pack(fill='x', expand=True)

password_entry = ttk.Entry(interface_config, textvariable=password, show="*")
password_entry.pack(fill='x', expand=True)

# IP adress
ip = ttk.Label(interface_config, text="IP:")
ip.pack(fill='x', expand=True)

password_entry = ttk.Entry(interface_config, textvariable=ipadr)
password_entry.pack(fill='x', expand=True)

# Broker port
Broker_port = ttk.Label(interface_config, text="Broker port:")
Broker_port.pack(fill='x', expand=True)

Broker_port_entry = ttk.Entry(interface_config, textvariable=broker_port)
Broker_port_entry.pack(fill='x', expand=True)

# topics 
Topic1 = ttk.Label(interface_config, text="Topic1:")
Topic1.pack(fill='x', expand=True)

Topic1_entry = ttk.Entry(interface_config, textvariable=topic)
Topic1_entry.pack(fill='x', expand=True)

# Config button
config_button = ttk.Button(interface_config, text="Configuration", command=Configuration_clicked)
config_button.pack(side = LEFT, fill='x', expand=True, pady=10)

# Entry with the COM port detected
texte = tk.Entry(interface_config)
comlist = serial.tools.list_ports.comports()
connected = []
for element in comlist:
    if((element.pid==29987)&(element.vid==6790)&(element.manufacturer=='wch.cn')):
        texte.insert(0,element.name)
texte.config(state=DISABLED)
texte.pack(side=RIGHT)

# Path button
Path_button = ttk.Button(interface_config, text="...", command=File_path_clicked)
Path_button.pack(side = RIGHT)

root_config.mainloop()