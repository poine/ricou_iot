#!/usr/bin/env python3
#-*- coding: utf-8 -*-
import paho.mqtt.client as mqtt, struct
from influxdb import InfluxDBClient
from typing import NamedTuple
import pdb


#INFLUXDB_ADDRESS = 'nhop.lan'
INFLUXDB_ADDRESS = 'ella.lan'
INFLUXDB_USER = 'root'
INFLUXDB_PASSWORD = 'root'
INFLUXDB_DATABASE = 'ricou_plants'
#INFLUXDB_DATABASE = 'ricou_iot'

PLANT_NB = 4

'''
Manipulate our influxdb database
'''

class SensorData(NamedTuple):
    location: str
    measurement: str
    value: float

class MyApp:
    def __init__(self, create_db=False):
        print(f'database host     : {INFLUXDB_ADDRESS}')
        print(f'         user/pass: {INFLUXDB_USER}/{INFLUXDB_PASSWORD}')
        self.influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)
        databases = self.influxdb_client.get_list_database()
        print(f'available databases:\n {databases}')
        if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
            if create_db:
                print('creating database')
                self.influxdb_client.create_database(INFLUXDB_DATABASE)
        else:
            print('found database')
        self.influxdb_client.switch_database(INFLUXDB_DATABASE)
        
        
    def _write_to_db(self, sensor_data):
        json_body = [{'measurement': _d.measurement,
                      'tags': {'location': _d.location},
                      'fields': {'value': _d.value}} for _d in sensor_data]     
        self.influxdb_client.write_points(json_body)
        
    def run(self):
        #self.mqtt_client.loop_forever()
        pass

    
if __name__ == "__main__":
    MyApp().run()

