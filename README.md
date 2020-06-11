# ricou_iot

Putting sensors in my home

# FIXME
plant_logger does not handle network down/up

## Installation

### Dependencies
  * Mosquitto


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

## Building Firmware

   * start arduino IDE
      /home/poine/work/arduino-1.8.12/arduino
