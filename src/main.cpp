#include <Arduino.h>
#include <secret.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <EEPROM.h>

WiFiClient espClient;
PubSubClient client(espClient);
#define EEPROM_SIZE 1

int LED_BUILTIN = 2;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char *mqtt_server = SECRET_MQTT_SERVER;
const char *mqtt_username = SECRET_MQTT_USERNAME;
const char *mqtt_password = SECRET_MQTT_PASSWORD;

const char *TOPIC_SUBSCRIBE = "esp32/action";
const char *TOPIC_PUBLISH_MESSAGE = "esp32/message";
const char *PUBLISH_CONNECTED = "connected";

long lasttime = millis();
long currentMillis = millis();
long prev_10ms = millis();
long prev_100ms = millis();
long prev_1s = millis();
long prev_2s = millis();
long prev_5s = millis();
long prev_10s = millis();
boolean ledOn = false;

void reconnect_wifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{

  String sPayload;

  for (int i = 0; i < length; i++)
  {
    sPayload += (char)payload[i];
  }

  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    EEPROM.write(0, 1);
  }
  else if ((char)payload[0] == '0')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED off by making the voltage HIGH
    EEPROM.write(0, 0);
  }
  else if (sPayload == "234")
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
    EEPROM.write(0, 1);
  }
  else
  {
  }
  EEPROM.commit();
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Start");

  reconnect_wifi();

  ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });

  ArduinoOTA.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  EEPROM.begin(EEPROM_SIZE);
  int ledState = EEPROM.read(0);
  digitalWrite(LED_BUILTIN, ledState);
}

void reconnect()
{
  // make sure wifi is connected
  reconnect_wifi();

  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "mqtt_";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(TOPIC_PUBLISH_MESSAGE, PUBLISH_CONNECTED);
      // ... and resubscribe
      client.subscribe(TOPIC_SUBSCRIBE);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void action_10s()
{
  if (!client.connected())
  {
    reconnect();
  }
}
void action_5s() {}

void action_2s()
{
  //  digitalWrite(LED_BUILTIN, ledOn);
  //  ledOn = !ledOn;
}

void action_1s()
{
  client.loop();
}
void action_100ms() {}

void action_10ms() {}

void loop()
{

  ArduinoOTA.handle();

  currentMillis = millis();
  if ((currentMillis - prev_10s) >= 10000)
  {
    prev_10s = currentMillis;
    action_10s();
  }
  if ((currentMillis - prev_5s) >= 5000)
  {
    prev_5s = currentMillis;
    action_5s();
  }
  if ((currentMillis - prev_2s) >= 2000)
  {
    prev_2s = currentMillis;
    action_2s();
  }
  if ((currentMillis - prev_1s) >= 1000)
  {
    prev_1s = currentMillis;
    action_1s();
  }
  if ((currentMillis - prev_100ms) >= 100)
  {
    prev_100ms = currentMillis;
    action_100ms();
  }
  if ((currentMillis - prev_10ms) >= 10)
  {
    prev_10ms = currentMillis;
    action_10ms();
  }
}