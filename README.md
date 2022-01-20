# ESP32 IMU MQTT Sensor

### Purpose
To stream data from an MPU6050 IMU to detect movement during sleep.

Have a button to start/stop the streaming of data.

Reacts to messages published on the device base topic. Ideally these should be sticky/retained so that the device can resume operation even if it crashes and restarts.

### Usage

Using the device `DOIT ESP32 DEVKIT V1` in Arduino IDE.

Copy `wifi_config.example.h` to `wifi_config.h` and configure accordingly. Root CA certificate can be pasted in PEM format (see example) and is needed to verify the authenticity of the broker the device is connecting to.

Libraries & Resources:
- [Adafruit MPU6050](https://github.com/adafruit/Adafruit_MPU6050) + dependencies
- [ESP32 DEVKIT V1 - DOIT Pinout](https://i1.wp.com/randomnerdtutorials.com/wp-content/uploads/2018/08/ESP32-DOIT-DEVKIT-V1-Board-Pinout-36-GPIOs-updated.jpg)
- [knolleary/pubsubclient](https://github.com/knolleary/pubsubclient)
- [knolleary/pubsubclient #462](https://github.com/knolleary/pubsubclient/issues/462)
- [programmer131/ESP8266-gBridge-TLS](https://github.com/programmer131/ESP8266-gBridge-TLS)
- [arduino-esp32 WifiClientSecure example](https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/examples/WiFiClientSecure/WiFiClientSecure.ino)