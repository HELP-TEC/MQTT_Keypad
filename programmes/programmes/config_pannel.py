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
comlist = serial.tools.list_ports.comports()
serial_name = "error"
ret = []
for element in comlist:
    if((element.pid==29987)&(element.vid==6790)&(element.manufacturer=='wch.cn')):
        serial_name=element.name
        
def int_to_bytes(val):
    retHex1=0
    retByte1=0
    retByte1=0
    if(val>pow(16,3)):
        retHex1=val//pow(16,3)
        val=val-retHex1*pow(16,3)
        retHex2 = val//pow(16,2)
    else:
        retHex2=val//pow(16,2)
    retByte1 = retHex1*16+retHex2
    retByte2 = val-retHex2*pow(16,2)
    return retByte1,retByte2
    

def fill_config_entry(root_to_close,filepath):
    fpath=pathlib.Path(filepath.get())
    print(fpath)
    found_file=os.path.exists(fpath)
    if(found_file&(fpath.suffix=='.json')):
        json_file = open("formatJSON.json", "r")
        jsondic = json.loads(json_file.read())
        username_entry.delete(0,END)
        password_entry.delete(0,END)
        ipadr_entry.delete(0,END)
        Broker_port_entry.delete(0,END)
        Topic1_entry.delete(0,END)
        username_entry.insert(0,jsondic['pannel_config_args'].get('MQTT_username'))
        password_entry.insert(0,jsondic['pannel_config_args'].get('Password'))
        ipadr_entry.insert(0,jsondic['pannel_config_args'].get('IP'))
        Broker_port_entry.insert(0,jsondic['pannel_config_args'].get('Broker_port'))
        Topic1_entry.insert(0,jsondic['pannel_config_args'].get('Topic1'))
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
    

def start_transfert_config():
    """ callback when the config button clicked
    """
    usr = username.get()
    pswd = password.get()
    pannel_ip = ipadr.get()
    port =  broker_port.get()
    MQTT_topic = topic.get()
    if (serial_name=="error"):
        messagebox.showerror("ERROR", "COM PORT isn't detected")
    else:
        Com_port_write(usr,pswd,pannel_ip,port,MQTT_topic)
    
    
def open_window_file_path():
    
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
    config_button = ttk.Button(interface_path, text="OK", command=lambda:fill_config_entry(root_config_fpath,filepath))
    config_button.pack(side = RIGHT)
    
    root_config_fpath.mainloop()


def Com_port_read():
    ser = serial.Serial(port= serial_name, baudrate=115200,timeout=1) 
    ser.close()
    ser.open()
    ser.close()
    print(1)

def Com_port_write(usr,pswd,pannel_ip,port,MQTT_topic):
    ser = serial.Serial(port= serial_name, baudrate=115200,timeout=1)
    ser.close()
    ser.open()
    payloadsize = len(usr)+len(pswd)+len(pannel_ip)+len(port)+1*len(MQTT_topic)
    print(payloadsize)
    if(payloadsize>255):
        [p1,p2] = int_to_bytes(payloadsize)
        print('p1 , p2',p1,p2)
        globalstr = chr(0).encode('latin_1')+chr(p1).encode('latin_1')+chr(p2).encode('latin_1')+usr.encode('latin_1')+pswd.encode('latin_1')+pannel_ip.encode('latin_1')+port.encode('latin_1')+MQTT_topic.encode()
    else:
        globalstr = chr(0).encode('latin_1')+chr(0).encode('latin_1')+chr(payloadsize).encode('latin_1')+usr.encode('latin_1')+pswd.encode('latin_1')+pannel_ip.encode('latin_1')+port.encode('latin_1')+MQTT_topic.encode()
    print(globalstr)
    print('-------------------------')
    ser.write(globalstr)
    print(ser.read(payloadsize+2))
    ser.close()

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

ipadr_entry = ttk.Entry(interface_config, textvariable=ipadr)
ipadr_entry.pack(fill='x', expand=True)

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
config_button = ttk.Button(interface_config, text="Configuration", command=start_transfert_config)
config_button.pack(side = LEFT, fill='x', expand=True, pady=10)

# Entry with the COM port detected
texte = tk.Entry(interface_config)
texte.insert(0,serial_name)
texte.config(state=DISABLED)
texte.pack(side=RIGHT)

# Path button
Path_button = ttk.Button(interface_config, text="...", command=open_window_file_path)
Path_button.pack(side = RIGHT)

root_config.mainloop()