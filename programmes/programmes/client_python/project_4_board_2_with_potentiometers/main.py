import paho.mqtt.client as mqtt #import the client1
import pyads
import threading
import time

#value to update led only when change occurse
# pannel 1
old_value=0
# pannel 2
old_value2=0
# pannel 3
old_value3=0
# pannel 4
old_value4=0

#Fonction apelée lorsque le brocke publish une valeur (Topic auquel on a subscribe)
def on_message(client, userdata, message):
    print("message");
    a=str(message.payload.decode("utf-8"))
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)

    ##################################################
    #ricive an update from a first card
    # panel 1
    ##################################################
    #si c'est la valeur des boutons réécrire sur le plc sur input
    if(message.topic == "topic/buttons"):
        plc.write_by_name("MAIN.input",int(a))#int.from_bytes(a,"little"))
    #si c'est la valeur du potentiomètre réécrire sur le plc sur output
    if (message.topic == "topic/potentiometer"):
        plc.write_by_name("MAIN.potentiometre",int(a))


    ##################################################
    # ricive an update from a second card
    # panel 2
    ##################################################
    # si c'est la valeur des boutons réécrire sur le plc sur input
    if(message.topic == "topic/buttons2"):
        plc.write_by_name("MAIN.input2",int(a))#int.from_bytes(a,"little"))
    # si c'est la valeur du potentiomètre réécrire sur le plc sur output
    if (message.topic == "topic/potentiometer2"):
        plc.write_by_name("MAIN.potentiometre2",int(a))


    ##################################################
    # ricive an update from a third card
    # panel 3
    # no potentiometer
    ##################################################
    # si c'est la valeur des boutons réécrire sur le plc sur input
    if(message.topic == "topic/buttons3"):
        plc.write_by_name("MAIN.input3",int(a))#int.from_bytes(a,"little"))

    ##################################################
    # ricive an update from a fourth card
    # panel 4
    # no potentiometer
    ##################################################
    # si c'est la valeur des boutons réécrire sur le plc sur input
    if(message.topic == "topic/buttons4"):
        plc.write_by_name("MAIN.input4",int(a))#int.from_bytes(a,"little"))



#prévien lors d'une déconexion
def on_disconnect(client, userdata, rc):
    if rc != 0:
        print("Unexpected MQTT disconnection. Will auto-reconnect")
#lors d'une reconnection ou d'une conection subscribe au bons topics
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    ##################################################
    # subscribe to a first card
    # panel 1
    ##################################################
    print("Subscribing to topic","topic/buttons")
    client.subscribe("topic/buttons",qos=1)

    print("Subscribing to topic","topic/potentiometer")
    client.subscribe("topic/potentiometer",qos=1)


    ##################################################
    # subscribe to a second card
    # panel 2
    ##################################################
    print("Subscribing to topic","topic/buttons2")
    client.subscribe("topic/buttons2",qos=1)

    print("Subscribing to topic","topic/potentiometer2")
    client.subscribe("topic/potentiometer2",qos=1)

    ##################################################
    # subscribe to a third card
    # panel 3
    # no potentiometer
    ##################################################
    print("Subscribing to topic","topic/buttons3")
    client.subscribe("topic/buttons3",qos=1)

    ##################################################
    # subscribe to a third card
    # panel 3
    # no potentiometer
    ##################################################
    print("Subscribing to topic","topic/buttons4")
    client.subscribe("topic/buttons4",qos=1)

#tread verifiant periodiquement la valeur des leds (si changement met à jour le topic sur le broker
def thread_function(name):
    while(1) :

        ###########################
        # var definition
        #########################

        # pannel 1
        global old_value
        # panel 2
        global old_value2
        # panel 3
        global old_value3
        # panel 4
        global old_value4

        ###########################
        # read plc values
        ###########################

        # pannel 1
        c = plc.read_by_name("MAIN.output")
        # panel 2
        c2 = plc.read_by_name("MAIN.output2")
        # panel 3
        c3 = plc.read_by_name("MAIN.output3")
        # panel 4
        c4 = plc.read_by_name("MAIN.output4")

        ###########################
        # update topic if value change
        ###########################

        # pannel 1
        if(c!=old_value):
            old_value=c
            client.publish("topic/leds", c,qos=1)  # publish
            print("publish var=", c)

        # pannel 2
        if(c2!=old_value2):
            old_value2=c2
            client.publish("topic/leds2", c2,qos=1)  # publish
            print("publish var=", c2)

        # pannel 3
        if(c3!=old_value3):
            old_value3=c3
            client.publish("topic/leds3", c3,qos=1)  # publish
            print("publish var=", c3)

        # pannel 4
        if(c4!=old_value4):
            old_value4=c4
            client.publish("topic/leds4", c4,qos=1)  # publish
            print("publish var=", c4)

        time.sleep(0.05) # periodic plc values pooling




#connection au plc
plc=pyads.Connection('127.0.0.1.1.1', pyads.PORT_TC3PLC1)
plc.open()


broker_address="127.0.0.1"
client = mqtt.Client("P1") #create new instance
client.username_pw_set("PyClient2", "CBPlcbB2046HPLPYWNFT")
client.on_message=on_message #attach function to callback
client.on_disconnect = on_disconnect
client.on_connect = on_connect
print("connecting to broker")
client.connect(broker_address) #connect au broker
x = threading.Thread(target=thread_function, args=(1,), daemon=True)
x.start()
client.loop_forever() #start
x.join()


