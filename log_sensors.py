#!/usr/bin/env python3
#-*- coding: utf-8 -*-
import paho.mqtt.client as mqtt, struct
from influxdb import InfluxDBClient
from typing import NamedTuple
import pdb

MQTT_CLIENT_ID = 'PythonLogger'
MQTT_TOPIC_PLANT = 'ricou/plant'
MQTT_TOPIC_WATERING = 'ricou/watering/terrasse_s/status'
#MQTT_ADDRESS = 'nhop.lan'
MQTT_ADDRESS = 'ella.lan'
MQTT_PORT = 1883

#INFLUXDB_ADDRESS = 'nhop.lan'
INFLUXDB_ADDRESS = 'ella.lan'
INFLUXDB_USER = 'root'
INFLUXDB_PASSWORD = 'root'
#INFLUXDB_DATABASE = 'ricou_plants'
INFLUXDB_DATABASE = 'ricou_iot'

PLANT_NB = 4

'''
Listen to plant messages on MQTT and stores their content in an influxdb database
FIXME: doesn't handle database disconnections
'''

class SensorData(NamedTuple):
    location: str
    measurement: str
    value: float

class MyApp:
    def __init__(self):
        self.mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.connect(MQTT_ADDRESS, MQTT_PORT)
        print(f'connected to mqtt: {MQTT_ADDRESS}:{MQTT_PORT}')
        for i in range(PLANT_NB):
            self.mqtt_client.subscribe(MQTT_TOPIC_PLANT+'/{}'.format(i+1))
        self.mqtt_client.subscribe(MQTT_TOPIC_WATERING) # watering

        self.influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)
        print(f'connected to database host: {INFLUXDB_ADDRESS}:8086 as {INFLUXDB_USER}:{INFLUXDB_PASSWORD}')
        databases = self.influxdb_client.get_list_database()
        print(f'existing databases: {databases}')
        if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
            print(f'creating empty database: {INFLUXDB_DATABASE}')
            self.influxdb_client.create_database(INFLUXDB_DATABASE)
        else:
            print(f'found database: {INFLUXDB_DATABASE}')
        self.influxdb_client.switch_database(INFLUXDB_DATABASE)
        
    def on_message(self, client, userdata, msg):
        #print('topic {}: ({}) {}'.format(msg.topic, len(msg.payload), str(msg.payload)))
        try:
            if msg.topic.startswith(MQTT_TOPIC_PLANT):
                datas = self._parse_plant_msg(msg)
                self._write_to_db(datas)
            elif msg.topic.startswith(MQTT_TOPIC_WATERING):
                datas = self._parse_watering_msg(msg)
                self._write_to_db(datas)
            else:
                print('topic {}: ({}) {}'.format(msg.topic, len(msg.payload), str(msg.payload)))
        except:
            print('parse error: topic {}: ({}) {}'.format(msg.topic, len(msg.payload), str(msg.payload)))
        
    def _parse_plant_msg(self, msg):
        location = msg.topic
        vals = struct.unpack('fffHIf', msg.payload)
        #print('{}: lux {:.1f} temp {:.1f} hum {:.1f} soil {:d} salt {:d} bat {:.1f}'.format(location, *vals))
        datas = [SensorData(location, meas, val) for meas, val in zip(['lux', 'temp', 'hum', 'soil', 'salt', 'bat'], vals)]
        return datas

    def _parse_watering_msg(self, msg):
        location = msg.topic
        vals = struct.unpack('f', msg.payload)
        print('{}: temp {:.1f}'.format(location, *vals))
        datas = [SensorData(location, meas, val) for meas, val in zip(['temp'], vals)]
        return datas

    def _write_to_db(self, sensor_data):
        json_body = [{'measurement': _d.measurement,
                      'tags': {'location': _d.location},
                      'fields': {'value': _d.value}} for _d in sensor_data]     
        self.influxdb_client.write_points(json_body)
        
    def run(self):
        self.mqtt_client.loop_forever()

    
if __name__ == "__main__":
    MyApp().run()

