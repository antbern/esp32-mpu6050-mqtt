#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const int led = LED_BUILTIN;

#include "wifi_config.h"


// Wifi and MQTT objects
WiFiClientSecure esp_client;
PubSubClient client(esp_client);

Adafruit_MPU6050 mpu;

// used to send messages after interval
unsigned long lastMsg = 0;

// the default interval between measurements
unsigned long interval = 5000;

volatile bool enabled = false;

// the buffer used to format the JSON output to send
#define MSG_BUFFER_SIZE  (256)
char msg[MSG_BUFFER_SIZE];

// the base topic to publish and listen to
#define TOPIC_BASE "esp210"

// using crude way for compile-time string concatenation using feature of the C89-compatible compiler
const char* topic_enabled = TOPIC_BASE "/enabled";
const char* topic_interval = TOPIC_BASE "/interval";
const char* topic_observation = TOPIC_BASE "/observation";

void setup() {
  // Start Serial
  Serial.begin(115200);

  // Set up pins
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  start_wifi();

  // TODO: connect to sensor
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while(1){
      digitalWrite(led, HIGH);
      delay(100);
      digitalWrite(led, LOW);
      delay(100);
    }
  }
  Serial.println("MPU6050 Found!");

  // configure device
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // configure the WifiClientSecure with the root CA
  esp_client.setCACert(mqtt_root_ca);

   // setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);


  // all done
  digitalWrite(led, LOW);

}

void start_wifi() {
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
    
    Serial.print(".");
  }
  // Print Debug info
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID

    // Attempt to connect
    if (client.connect(mqtt_device_name, mqtt_username, mqtt_password)) {
      Serial.println("connected");

      // do stuff when connected here (like subscribing to topics)
      
      client.subscribe(topic_interval);
      client.subscribe(topic_enabled);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < len; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // convert topic and payload to strings
  String topicstr(topic);

  // add null-termination in last place (ok as long as we are not near the end of the buffer this points to)
  payload[len] = '\0';
  String payloadstr((char*)payload);

  // try to interpret the message as an unsigned long
  long i = payloadstr.toInt();
  
  if (topicstr.endsWith("interval")) {
    if (i >= 500) {
      // we managed to parse an unsigned long, use it as interval for now
      Serial.print("New interval received: ");
      Serial.println(i, DEC);
      
      interval = i;
    }
  } else if (topicstr.endsWith("enabled")) {
    enabled = i > 0 ? true : false;
    Serial.print("Set enabled: ");
    Serial.println(enabled);
  }
}

// used later for turning on/off with a physical button on the device
void setEnabled(bool enabled) {
  client.publish(topic_enabled, (const char*)&(enabled ? "1" : "0"), true);  
}

void loop() {
  if(!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // don't continue if we are not supposed to acquire and publish anything
  if(!enabled){
    return;
  }

  unsigned long now = millis();
  if(now - lastMsg > interval) {
    lastMsg = now;

    // read data
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);


    // format it to JSON
    snprintf(msg, MSG_BUFFER_SIZE, "{\"acceleration\":{\"x\":%.3f,\"y\":%.3f,\"z\":%.3f}, \"gyro\":{\"x\":%.3f,\"y\":%.3f,\"z\":%.3f}, \"temperature\":%.3f}", a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z, temp.temperature);
    
    Serial.print("Publish message: ");
    Serial.println(msg);

    

    // publish message
    client.publish(topic_observation, msg);  

  }

}
