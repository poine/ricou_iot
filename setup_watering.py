#!/usr/bin/env python3
#-*- coding: utf-8 -*-
import argparse, yaml
import paho.mqtt.client as mqtt, struct
#from influxdb import InfluxDBClient
from typing import NamedTuple
import pdb

MQTT_ADDRESS = 'ella.lan'
MQTT_PORT = 1883
MQTT_CLIENT_ID = 'SetupWatering'
MQTT_TOPIC = 'ricou/watering/terrasse_s'
MQTT_TOPIC_GET_SCHEDULE = 'ricou/watering/terrasse_s/schedule'
MQTT_TOPIC_SET_SCHEDULE = 'ricou/watering/terrasse_s/setup'
MQTT_TOPIC_OVERRIDE = 'ricou/watering/terrasse_s/override'
MQTT_TOPIC_FORCE_STOP = 'ricou/watering/terrasse_s/stop'

'''
'''

class SensorData(NamedTuple):
    location: str
    measurement: str
    value: float


# How do I get that automagically? (from libwatering_ctl.h)
# #define MAX_WORKING_PERIOD 16
# struct SchedulerParam {
# uint16_t nb_period;
# uint16_t start_min[MAX_WORKING_PERIOD];
# uint16_t dur_min[MAX_WORKING_PERIOD];
# };
class Schedule:
    def __init__(self):
        self.nb_periods = 0
        self.start_min, self.dur_min = [], []

    def parse_binary(self, payload):
        self.nb_periods = struct.unpack('H', payload[:2])[0]
        fmt = ''.join(['H' for _i in range(self.nb_periods)])
        p0 = 2; p1 = p0+2*self.nb_periods
        self.start_min = struct.unpack(fmt, payload[p0:p1])
        p0 = 2+2*16; p1 = p0+2*self.nb_periods
        self.dur_min = struct.unpack(fmt, payload[p0:p1])

    def to_binary(self):
        pad = [0]*(16-self.nb_periods)
        _d = [self.nb_periods]+self.start_min+pad+self.dur_min+pad
        fmt = ''.join(['H' for _i in range(1+2*16)])
        payload = struct.pack(fmt, *_d)
        #print(payload)
        return payload

    def save_yaml(self, filename):
        txt = 'periods:\n'
        for i in range(self.nb_periods):
            txt += f'  {i}:\n    start_min: {self.start_min[i]}\n    dur_min: {self.dur_min[i]}\n'
        with open(filename, 'w') as f:
            f.write(txt)
            print(f'saved schedule to {filename}')

    def read_yaml(self, filename):
        with open(filename) as f:
            _dict = yaml.load(f, Loader=yaml.SafeLoader)
            print(f'read schedule from {filename}')
        _start_min, _dur_min = [], []
        for _k, _args in _dict.items():
            if _k == 'periods':
                for __k, __args in _dict[_k].items():
                    _start_min.append(__args['start_min'])
                    _dur_min.append(__args['dur_min'])
        self.start_min, self.dur_min = _start_min, _dur_min
        self.nb_periods = len(_start_min)
    
    def print(self):
        print('schedule')
        print(f'  periods: {self.nb_periods}')
        for i in range(self.nb_periods):
            _s = _min_to_hms(self.start_min[i])
            _e = _min_to_hms(self.start_min[i]+self.dur_min[i])
            print(f'     {_s} -> {_e}')
            
def _min_to_hms(m):
    _h, _m = divmod(m, 60)
    return f'{_h:02d}h{_m:02d}m'
        
        
class MyApp:
    def __init__(self):
        self.mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.connect(MQTT_ADDRESS, MQTT_PORT)

    def on_message(self, client, userdata, msg):
        print('topic {}: ({}) {}'.format(msg.topic, len(msg.payload), str(msg.payload)))
        #try:
        datas = self._parse_msg(msg)
        #self._write_to_db(datas)
        #self.mqtt_client.publish('ricou/watering/terrasse_s/setup', "Hello world!");
        #except:
        #    print('parse error')
        
    def __parse_msg(self, msg):
        #print(msg)
        #location = msg.topic
        #vals = struct.unpack('fffHIf', msg.payload)
        #print('{}: lux {:.1f} temp {:.1f} hum {:.1f} soil {:d} salt {:d} bat {:.1f}'.format(location, *vals))
        #datas = [SensorData(location, meas, val) for meas, val in zip(['lux', 'temp', 'hum', 'soil', 'salt', 'bat'], vals)]
        return None#datas
    
        
    # def _write_to_db(self, sensor_data):
    #     json_body = [{'measurement': _d.measurement,
    #                   'tags': {'location': _d.location},
    #                   'fields': {'value': _d.value}} for _d in sensor_data]     
    #     self.influxdb_client.write_points(json_body)


    def _parse_msg(self, msg):
        if msg.topic == MQTT_TOPIC_GET_SCHEDULE:
            s = Schedule()
            s.parse_binary(msg.payload)
            s.print()
            s.save_yaml('/tmp/sched2.yaml')
            self.mqtt_client.disconnect()
        return None


    
    def run(self, cmd, opt):
        if cmd == 'r':   # read schedule from device
            self.mqtt_client.subscribe(MQTT_TOPIC_GET_SCHEDULE)
        elif cmd == 'w': # write schedule to device
            s = Schedule()
            s.read_yaml('/tmp/sched.yaml')
            s.print()
            self.mqtt_client.publish(MQTT_TOPIC_SET_SCHEDULE, s.to_binary());
            print(f'sent schedule to device {MQTT_TOPIC_SET_SCHEDULE}')
        elif cmd == 'o': # override schedule
            print(opt)
            payload = '' if len(opt)==0 else int(opt[0])
            self.mqtt_client.publish(MQTT_TOPIC_OVERRIDE, payload);
            print(f'sent override to device {MQTT_TOPIC_OVERRIDE}')
        elif cmd == 's': # forced stop
            payload = '' if len(opt)==0 else opt[0]
            self.mqtt_client.publish(MQTT_TOPIC_FORCE_STOP, payload);
            print(f'sent override to device {MQTT_TOPIC_FORCE_STOP}')
        
        self.mqtt_client.loop_forever()

    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Setup Watering.')
    parser.add_argument('opt_arg', nargs='*')
    parser.add_argument('-c', '--cmd', default='r', help='what to do: r, w, o, s')
    args = parser.parse_args()
    MyApp().run(args.cmd, args.opt_arg)

