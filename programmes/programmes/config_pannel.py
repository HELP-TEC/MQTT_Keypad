# -*- coding: utf-8 -*-
"""
Created on Wed Sep 28 14:35:29 2022
    Script to run to config Pannel
@author: louis
"""

import tkinter as tk
import serial.tools.list_ports
from subprocess import Popen, PIPE
from tkinter import *
import time
import xlrd, xlwt, xlutils
from xlutils.copy import copy
import os
from threading import Thread


import tkinter as tk
from tkinter import ttk
from tkinter.messagebox import showinfo
from tkinter.messagebox import *

# root window
root = tk.Tk()
root.geometry("500x500")
root.resizable(False, False)
root.title('Pannel configuration')

# store email address and password
username = tk.StringVar()
password = tk.StringVar()
ipadr = tk.StringVar()
broker_port = tk.StringVar()
topic = tk.StringVar()


def Configuration_clicked():
    """ callback when the login button clicked
    """
    usr = username.get()
    pswd = password.get()
    
    print(username)
    
def Path_file_clicked():
    print(1)
    

# frame
Interface = ttk.Frame(root)
Interface.pack(padx=10, pady=10, fill='x', expand=False)


# Config part
Config = ttk.Label(Interface, text="Config:")
Config.pack(fill='x', expand=False)

# email
MQTT_Username = ttk.Label(Interface, text="MQTT_Username:")
MQTT_Username.pack(fill='x', expand=True)

username_entry = ttk.Entry(Interface, textvariable=username)
username_entry.pack(fill='x', expand=True)
username_entry.focus()

# password
MQTT_password = ttk.Label(Interface, text="Password:")
MQTT_password.pack(fill='x', expand=True)

password_entry = ttk.Entry(Interface, textvariable=password, show="*")
password_entry.pack(fill='x', expand=True)

# IP
IP = ttk.Label(Interface, text="IP:")
IP.pack(fill='x', expand=True)

password_entry = ttk.Entry(Interface, textvariable=ipadr)
password_entry.pack(fill='x', expand=True)

# Broker port
Broker_port = ttk.Label(Interface, text="Broker port:")
Broker_port.pack(fill='x', expand=True)

Broker_port_entry = ttk.Entry(Interface, textvariable=broker_port)
Broker_port_entry.pack(fill='x', expand=True)

# topics
Topic1 = ttk.Label(Interface, text="Topic1:")
Topic1.pack(fill='x', expand=True)

Topic1_entry = ttk.Entry(Interface, textvariable=password)
Topic1_entry.pack(fill='x', expand=True)

# Config button
config_button = ttk.Button(Interface, text="Configuration", command=Configuration_clicked)
config_button.pack(side = LEFT, fill='x', expand=True, pady=10)

# Config button
Path_button = ttk.Button(Interface, text="...", command=Path_file_clicked)
Path_button.pack(side = RIGHT)

root.mainloop()