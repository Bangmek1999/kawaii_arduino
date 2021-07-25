import paho.mqtt.client as mqtt
import queue
from threading import Thread
import ssl
from datetime import datetime,date
import time
import pymysql
Q=queue.Queue()

def on_connect(client,userdata,flag,rc):
    print("Connected! ",rc)
    client.subscribe("kawaii/#")
def on_message(client,userdata,msg):
    topic=msg.topic
    message=msg.payload.decode('utf-8')
    #print(topic,message)
    Q.put((topic,message))
client=mqtt.Client()
client.on_connect=on_connect
client.on_message=on_message
client.username_pw_set('dev',password='dev123456789')
client.tls_set(certfile=None,keyfile=None,cert_reqs=ssl.CERT_REQUIRED,
               tls_version=ssl.PROTOCOL_TLSv1_2,ciphers=None)
client.tls_insecure_set(False)
#------------------------Database
def Logger():
    while(True):
        try:
            db=pymysql.connect(host="202.28.37.134",user="benzplant",
                               password="benz1234",db="Benz")
            cursor=db.cursor()
        except:
            print("\nDB down!")
            time.sleep(30)
            continue
        while(Q.empty()==0):
            (topic,message)=Q.get()
            topic_section=topic.split("/",-1)
            sensor_id=topic_section[2]
            data_section=message.split(",",-1)
            if topic == "kawaii/clients/%s/sync"%(sensor_id):
                print(topic,message)
            else:
                print(topic)
                print("take data")
                time_stamp=datetime.fromtimestamp(int(data_section[0]))
                sql="insert into rawdata (mac,time_stamp,light,DHT_temp,DHT_Humidity,EC1,EC2,EC3,dallas_temp1,dallas_temp2) values('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')"%(sensor_id,
                                                                               time_stamp,
                                                                               data_section[8],
                                                                               data_section[4],
                                                                               data_section[5],
                                                                               data_section[1],
                                                                               data_section[2],
                                                                               data_section[3],
                                                                               data_section[6],
                                                                               data_section[7],                                                                            
                                                                                     )
                print(sql)
                cursor.execute(sql)
                db.commit()
        db.close()            
        time.sleep(1)
        
worker1=Thread(target=Logger)
worker1.start()

#------------------------------------- MQTT Thread
while(True):
    client.loop()
    if(client.is_connected()==False):
        try:
            print("connect to MQTT")
            if(client.connect("mqtt.csmju.com",8883,60)==0):
                print("MQTT connected!")
            else:
                print("MQTT connection fail!")
            client.loop()
        except:
            print("MQTT server down!")
            time.sleep(60)
