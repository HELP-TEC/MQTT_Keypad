import paho.mqtt.client as mqtt #import the client1
import pyads
import threading
import time
#adresse ip du brocker
broker_address="127.0.0.1"
#def des nom des topics
topic_buton="topic/buttons"
topic_potentiometer="topic/potentiometer"
topic_led="topic/leds"
#autentification
user_name="PyClient2"
user_pass="CBPlcbB2046HPLPYWNFT"
#def des ref des var sur le plc
var_button_plc="MAIN.input"
var_potentiometer_plc="MAIN.potentiometre"
var_led_plc="MAIN.output"
#ip du plc
IP_PLC='127.0.0.1.1.1'
#var générale
old_value=0


#Fonction apelée lorsque le brocke publish une valeur (Topic auquel on a subscribe)
def on_message(client, userdata, message):
    print("message");
    a=str(message.payload.decode("utf-8"))
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
    #si c'est la valeur des boutons réécrire sur le plc sur input
    if(message.topic == topic_buton):
        plc.write_by_name(var_button_plc,int(a))#int.from_bytes(a,"little"))
    #si c'est la valeur du potentiomètre réécrire sur le plc sur output
    if (message.topic == topic_potentiometer):
        plc.write_by_name(var_potentiometer_plc,int(a)) 
    

#prévien lors d'une déconexion
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected MQTT disconnection. Will auto-reconnect")
#lors d'une reconnection ou d'une conection subscribe au bons topics
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    print("Subscribing to topic",topic_buton)
    client.subscribe(topic_buton,qos=1)
    print("Subscribing to topic",topic_potentiometer)
    client.subscribe(topic_potentiometer,qos=1)
#tread verifiant periodiquement la valeur des leds (si changement met à jour le topic sur le broker
def thread_function(name):
    while(1) :
        global old_value
        c = plc.read_by_name(var_led_plc)

        if(c!=old_value):
            old_value=c
            client.publish(topic_led, c,qos=1)  # publish
            print("publish var=", c)
        time.sleep(0.05)




#connection au plc
plc=pyads.Connection(IP_PLC, pyads.PORT_TC3PLC1)
plc.open()




#broker_address="iot.eclipse.org" #use external broker
client = mqtt.Client("P1") #create new instance

client.username_pw_set(user_name, user_pass)
#client.connect(broker_address) #connect to broker
#client.publish("topic/leds",0)#publish
client.on_message=on_message #attach function to callback si reseption publish
client.on_disconnect = on_disconnect #attach function to callback si déconection
client.on_connect = on_connect #attach function to callback si conection
print("connecting to broker")
client.connect(broker_address) #connect au broker
x = threading.Thread(target=thread_function, args=(1,), daemon=True)#generation tread
x.start()#generation tread
client.loop_forever() #start la loop du broker
x.join()#generation tread


