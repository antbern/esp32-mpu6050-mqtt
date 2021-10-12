#include <WiFi.h>
#include <PubSubClient.h>

const int led = LED_BUILTIN;

#include "wifi_config.h"


// Wifi and MQTT objects
WiFiClient esp_client;
PubSubClient client(esp_client);

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
    if (client.connect(mqtt_device_name)) {
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // convert topic and payload to strings
  String topicstr(topic);
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

    // read data: BH1750 first since it takes some time (~500ms)

    float lux = 3.0f;
    float temp = 312.0f;
    float pressure = 31.0f;
    float humidity = 32.0f;

    // format it to JSON
    snprintf(msg, MSG_BUFFER_SIZE, "{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f,\"illuminance\":%.2f}", temp, pressure, humidity, lux);
    
    Serial.print("Publish message: ");
    Serial.println(msg);

    // publish message
    client.publish(topic_observation, msg);  

  }

}
