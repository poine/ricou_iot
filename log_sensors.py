#!/usr/bin/env python3
#-*- coding: utf-8 -*-
import paho.mqtt.client as mqtt, struct
from influxdb import InfluxDBClient
from typing import NamedTuple
import pdb

MQTT_CLIENT_ID = 'PythonLogger'
MQTT_TOPIC = 'ricou/plant'
MQTT_ADDRESS = 'nhop.lan'
MQTT_PORT = 1883

INFLUXDB_ADDRESS = 'nhop.lan'
INFLUXDB_USER = 'root'
INFLUXDB_PASSWORD = 'root'
INFLUXDB_DATABASE = 'ricou_plants'
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
        for i in range(1,3):
            self.mqtt_client.subscribe(MQTT_TOPIC+'/{}'.format(i))

        self.influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)
        databases = self.influxdb_client.get_list_database()
        print(databases)
        if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
            print('creating database')
            self.influxdb_client.create_database(INFLUXDB_DATABASE)
        else:
            print('found database')
        self.influxdb_client.switch_database(INFLUXDB_DATABASE)
        
    def on_message(self, client, userdata, msg):
        #print('topic {}: ({}) {}'.format(msg.topic, len(msg.payload), str(msg.payload)))
        try:
            datas = self._parse_msg(msg)
            self._write_to_db(datas)
        except:
            print('parse error')
        
    def _parse_msg(self, msg):
        location = msg.topic
        vals = struct.unpack('fffHIf', msg.payload)
        print('{}: lux {:.1f} temp {:.1f} hum {:.1f} soil {:d} salt {:d} bat {:.1f}'.format(location, *vals))
        datas = [SensorData(location, meas, val) for meas, val in zip(['lux', 'temp', 'hum', 'soil', 'salt', 'bat'], vals)]
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

