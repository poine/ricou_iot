# ricou_iot

Putting sensors in my home

## FIXME
plant_logger does not handle network down/up

## Installation

### Dependencies
 
  sudo apt install python3-influxdb
  sudo apt install python3-paho-mqtt
  
  * Mosquitto
    sudo apt install mosquitto-clients mosquitto

  * Influxdb
	https://docs.influxdata.com/influxdb/v1.8/introduction/install/

  * Grafana
	https://grafana.com/docs/grafana/latest/installation/debian/


### Startup

   * sudo cp system/plant_logger.service /etc/systemd/system/
   * sudo chmod 644 /etc/systemd/system/plant_logger.service 
   * sudo systemctl daemon-reload
   * sudo systemctl status plant_logger.service
   * sudo systemctl start plant_logger.service
   * sudo systemctl enable plant_logger.service
   * journalctl -u plant_logger.service


   * systemctl list-units --type=service
   * sudo systemctl status grafana-server
   
   
## Building Firmware

   * start arduino IDE
      /home/poine/work/arduino-1.8.12/arduino


## Details
### influxdb
migrate:
on new server (after enabling bind-address on old one, see https://docs.influxdata.com/influxdb/v1.8/administration/backup_and_restore/):
influxd backup -portable -database ricou_plants -host nhop:8088 /tmp/ricou_plants-snapshot
influxd restore -portable /tmp/ricou_plants-snapshot

### mosquito

 mosquitto_sub -h nhop -p 1883 -t ricou/# -v
ricou/plant/4 ���@���A�A�?���E
ricou/plant/4 ���@�̸A�@�?��E
ricou/plant/4 ���@�A�@�?��E
ricou/plant/4 ���@�A�@�?y��E
ricou/plant/4 ���@ff�A�@�?���E
ricou/plant/4 ���@�̸A�@�?��E


### Grafana
https://grafana.com/docs/grafana/latest/installation/debian/
https://grafana.com/docs/grafana/latest/dashboards/export-import/
