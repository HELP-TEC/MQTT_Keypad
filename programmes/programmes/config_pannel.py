# -*- coding: utf-8 -*-
"""
Created on Wed Sep 28 14:35:29 2022
    Script to run to config Pannel
@author: louis
"""
import serial.tools.list_ports
import tkinter as tk
from tkinter import *
from tkinter import ttk
from tkinter import messagebox
import pathlib
import os
import json
import threading 
import time

# root_config window
root_config = tk.Tk()
root_config.geometry("400x280")
root_config.resizable(False, False)
root_config.title('Pannel configuration')
# Data to store
username = tk.StringVar()
password = tk.StringVar()
ipadr = tk.StringVar()
broker_port = tk.StringVar()
topic_vars = []
topic_label = []
topic_entry = []
kill_threads = 0
def check_com_port():
    while(1):
        global kill_threads,texte
        if(kill_threads):
            break;
        serial_name = "error"
        comlist = serial.tools.list_ports.comports()
        for element in comlist:
            if((element.pid==29987)&(element.vid==6790)&(element.manufacturer=='wch.cn')):
                serial_name=element.name
        texte.config(state="normal")
        if(texte.get()!=''):
            texte.delete(0,END)
        texte.insert(0,serial_name)
        texte.config(state="disabled")
        time.sleep(1)
def update_topic_entries():
    while(1):
        global topic_name,topic_entry,topic_vars,kill_threads
        if (kill_threads):
            break
        num = int(sp.get())
        if(num!=len(topic_entry)):
            topic_vars.clear()
            resize = 280+40*num
            root_config.geometry("400x"+str(resize))
            config_button.forget()
            path_button.forget()
            read_button.forget()
            texte.forget()
            for i in range(len(topic_entry)):
                topic_entry[i].forget()
                topic_label[i].forget()
            topic_entry.clear()
            topic_label.clear()
            for i in range(num):
                # topics
                topic_vars.append(tk.StringVar())
                topic_label.append(ttk.Label(interface_config, text="Topic " +str(i+1)+ " :",width = 10))
                topic_label[i].pack(fill='x', expand=True)
                topic_entry.append(ttk.Entry(interface_config, textvariable=topic_vars[i],width = 20))
                topic_entry[i].pack(fill='x', expand=True)
            config_button.pack(side = LEFT, fill='x', expand=True, pady=10)
            path_button.pack(side = LEFT, fill='x', expand=True, pady=10)
            texte.pack(side=RIGHT, fill='x', expand=True, pady=10)
            read_button.pack(side = LEFT, fill='x', expand=True, pady=10)
        time.sleep(0.05)
    
        
def int_to_2bytes(val):
    retHex1=0
    retByte1=0
    retByte2=0
    if(val>pow(16,3)):
        retHex1=val//pow(16,3)
        val=val-retHex1*pow(16,3)
        retHex2 = val//pow(16,2)
    else:
        retHex2=val//pow(16,2)
    retByte1 = retHex1*16+retHex2
    retByte2 = val-retHex2*pow(16,2)
    return retByte1,retByte2

def write_values_to_entries(jsondic):
        username_entry.delete(0,END)
        password_entry.delete(0,END)
        ipadr_entry.delete(0,END)
        broker_port_entry.delete(0,END)
        username_entry.insert(0,jsondic['pannel_config_args'].get('MQTT_username'))
        password_entry.insert(0,jsondic['pannel_config_args'].get('password'))
        ipadr_entry.insert(0,jsondic['pannel_config_args'].get('ip'))
        broker_port_entry.insert(0,jsondic['pannel_config_args'].get('broker_port'))
        for i in range(len(topic_entry)):
            if (('topic'+str(i)) in jsondic['pannel_config_args']):
                topic_entry[i].delete(0,END)
                print(jsondic['pannel_config_args'].get(('topic'+str(i))))
                topic_entry[i].insert(0,jsondic['pannel_config_args'].get(('topic'+str(i))))
                
        

def fill_config_entry(root_to_close,filepath):
    fpath=pathlib.Path(filepath.get())
    found_file=os.path.exists(fpath)
    if(found_file&(fpath.suffix=='.json')):
        json_file = open(fpath, "r")
        jsondic = json.loads(json_file.read())
        write_values_to_entries(jsondic)
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
    topic_names = []
    for i in topic_vars:
        topic_names.append(i.get())
    if (serial_name=="error"):
        messagebox.showerror("ERROR", "COM PORT isn't detected")
    else:
        if((usr=='')|(pswd=='')|(pannel_ip=='')|(port=='')|('' in topic_names)):
            messagebox.showerror("ERROR", "Current configuration have atleast one blank entry")
        else:  
            Com_port_write(usr,pswd,pannel_ip,port,topic_names)
    
    
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
    ok_button = ttk.Button(interface_path, text="OK", command=lambda:fill_config_entry(root_config_fpath,filepath))
    ok_button.pack(side = RIGHT)
    
    root_config_fpath.mainloop()


def Com_port_read():
    if(serial_name != "error"):
        ser = serial.Serial(port= serial_name, baudrate=115200,timeout=1) 
        ser.close()
        ser.open()
        ser.write(chr(1).encode('latin_1'))
        esp_config=ser.read(1024).decode()
        esp_config=esp_config[0:len(esp_config)-1]
        current_config = open('current_config.json','w')
        jsondic = json.loads(esp_config)
        json.dump(jsondic,current_config,indent=4)
        write_values_to_entries(jsondic)
        messagebox.showinfo("Config", "Read config done")
        ser.close()
    else:
        messagebox.showerror("ERROR", "COM PORT isn't detected")

def Com_port_write(usr,pswd,pannel_ip,port,topic_names):
    ser = serial.Serial(port= serial_name, baudrate=115200,timeout=1)
    ser.close()
    ser.open()
    dic_to_write = {'pannel_config_args': {'MQTT_username': usr,
                                           'password': pswd,
                                           'ip': pannel_ip,
                                           'broker_port':port}}
    for i in range(len(topic_names)):
        dic_to_write['pannel_config_args']['topic'+str(i)]=topic_names[i]
    data_to_write = json.dumps(dic_to_write,indent=4, 
                      separators=(',', ': '),
                      ensure_ascii=True)
    payloadsize = len(data_to_write)
    if(payloadsize>255):
        [p1,p2] = int_to_2bytes(payloadsize)
        print('p1 , p2',p1,p2)
        globalstr = chr(0).encode('latin_1')+chr(p1).encode('latin_1')+chr(p2).encode('latin_1')+data_to_write.encode('latin_1')
    else:
        globalstr = chr(0).encode('latin_1')+chr(0).encode('latin_1')+chr(payloadsize).encode('latin_1')+data_to_write.encode('latin_1')
    print(globalstr)
    print('-------------------------')
    ser.write(globalstr)
    ser.close()
    
# frame
interface_config = ttk.Frame(root_config)
interface_config.pack(padx=10, pady=10, fill='x', expand=False)

# Config part
Config = ttk.Label(interface_config, text="Config:",width = 10)
Config.pack(fill='x', expand=False)

# MQTT username
MQTT_username_label = ttk.Label(interface_config, text="MQTT_username:",width = 10)
MQTT_username_label.pack(fill='x', expand=True)
username_entry = ttk.Entry(interface_config, textvariable=username,width = 20)
username_entry.pack(fill='x', expand=True)
username_entry.focus() 

# MQTT password
MQTT_password_label = ttk.Label(interface_config, text="Password:",width = 10)
MQTT_password_label.pack(fill='x', expand=True)

password_entry = ttk.Entry(interface_config, textvariable=password, show="*",width = 20)
password_entry.pack(fill='x', expand=True)

# IP adress
ip_label = ttk.Label(interface_config, text="IP:",width = 10)
ip_label.pack(fill='x', expand=True)

ipadr_entry = ttk.Entry(interface_config, textvariable=ipadr,width = 20)
ipadr_entry.pack(fill='x', expand=True)

# Broker port
broker_port_label = ttk.Label(interface_config, text="Broker port:",width = 10)
broker_port_label.pack(fill='x', expand=True)

broker_port_entry = ttk.Entry(interface_config, textvariable=broker_port,width = 20)
broker_port_entry.pack(fill='x', expand=True)

# spinbox for the number of topics
spinbox_label = ttk.Label(interface_config, text="Number of topics:",width = 10)
spinbox_label.pack(fill='x', expand=True)

sp = tk.Spinbox(interface_config, from_=0, to=5,width = 20)
sp.pack(side=TOP)

# Config button
config_button = ttk.Button(interface_config, text="Configuration", command=start_transfert_config)
config_button.pack(side = LEFT, fill='x', expand=True, pady=10)

# Entry with the COM port detected
texte = tk.Entry(interface_config,width=10)
#texte.insert(0,serial_name)
# texte.config(state=DISABLED)
texte.pack(side=RIGHT, fill='x', expand=True, pady=10)

# Path button
path_button = ttk.Button(interface_config, text="...", command=open_window_file_path)
path_button.pack(side = LEFT, fill='x', expand=True, pady=10)

# Read button 
read_button = ttk.Button(interface_config, text="Read Config", command=Com_port_read)
read_button.pack(side = RIGHT, fill='x', expand=True, pady=10)

topic_thread = threading.Thread(target=update_topic_entries, daemon=True)
topic_thread.start()
com_thread = threading.Thread(target=check_com_port, daemon=True)
com_thread.start()
root_config.mainloop()
kill_threads = 1