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


// the buffer used to format the JSON output to send
#define MSG_BUFFER_SIZE  (256)
char msg[MSG_BUFFER_SIZE];

const char* topic_environment = "esp32/environment";
const char* topic_interval = "esp32/interval";

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

  // try to interpret the message as an unsigned long
  char *end;
  unsigned long i = strtoul((const char*)payload, &end, 10);

  if (end != (char*)payload && i >= 500) {
    // we managed to parse an unsigned long, use it as interval for now
    Serial.print("New interval received: ");
    Serial.println(i, DEC);
    
    interval = i;
  }
}

void loop() {
  if(!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

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
    client.publish(topic_environment, msg);  

  }

}
