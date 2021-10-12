# ESP32 Bed Motion MQTT Sensor

### Purpose
To stream IMU data from the bed to detect movement during sleep.

Have a button to start/stop the streaming of data.

Reacts to messages published on the device base topic. Ideally these should be sticky so that the device can resume operation even if it crashes and restarts.

### Usage

Using the device: DOIT ESP32 DEVKIT V1 in Arduino IDE

Copy `wifi_config.example.h` to `wifi_config.h` and configure accordingly.

