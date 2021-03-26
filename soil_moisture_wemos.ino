/************************************************
*   soil_moisture_wemos
*   gswann - 26 Mar 2021
*   
*   modified from adafruit example
* 
* 
* In HA "Services" mqtt.publish use this to set retain flag for this topic
* {"topic": "sm1/modereq","payload":"sleep","retain":true}
* and switch between going to sleep after a report, or staying 
* awake indefinitely.
*************************************************/


#include "Adafruit_seesaw.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>

#define VERSION "0.94"

#include "secrets.h"

//********************************
#define mqtt_user "hass"                //enter your MQTT username
#define mqtt_password "hass"            //enter your password
#define mqtt_client "client-soilmoisture1"    // must be unique for each board/client
//********************************

WiFiClient espClient;
PubSubClient client(espClient);

const int onesec = 1000;
unsigned long myMillis = 0;
unsigned long oldMillis = 0;

unsigned int mySecs = 55;
unsigned int myMins = 0;
unsigned int cntMR = 0;
bool SleepStatus = 0;    // 1 = going to sleep   0 = staying awake

Adafruit_seesaw ss;
const char* WiFi_hostname = "soilmoisture1";
const int led     = LED_BUILTIN; // blue LED on the ESP board
const int myLed   = LED_BUILTIN; // blue LED on the ESP board
const int warning = LED_BUILTIN; // blue LED on the ESP board

const char* mqtt_server = "192.168.2.6";

//********************************************
void setup() {
  Serial.begin(115200);

  Serial.println("seesaw Soil Sensor example!");

// turn on the seesaw
  pinMode(D6,OUTPUT); 
  digitalWrite(D6, HIGH);
  delay(2000);

  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw not found");
    while (1);
  } else {
    Serial.print("seesaw started! version: ");
    Serial.println(ss.getVersion(), HEX);
  }

  setupWifi();
  setupOTA();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // blink three quick for all ready
  for (int i = 0 ; i < 3; i++) {
    digitalWrite(myLed, LOW);
    delay(200);
    digitalWrite(myLed, HIGH);
    delay(100);
  }

  client.loop();
  Serial.println(F("Program startup complete"));
  Serial.println();
  Serial.println();

}

//********************************************
void loop() {

  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }

  float tempC = ss.getTemp();
  float tempF = ((tempC * 9) / 5) + 32;

  int inttempF = int(tempF);

  uint16_t capread = ss.touchRead(0);

  //  Serial.print("Temperature: "); Serial.print(tempC); Serial.println("*C");
  //  Serial.print("Temperature: "); Serial.print(tempF); Serial.println("*F");
  //  Serial.print("Capacitive: "); Serial.println(capread);
  //delay(1000);

  client.loop();

  myMillis = millis();
  if ((myMillis - oldMillis) >= onesec) {
    //    oldMillis = myMillis;
    oldMillis += onesec;
    mySecs += 1;
    if (mySecs >= 60) {
      mySecs -= 60;
      myMins += 1;

      Serial.print("Temperature: "); Serial.print(tempF); Serial.println("*F");
      Serial.print("Capacitive: "); Serial.println(capread);

      digitalWrite(myLed, LOW);

      Serial.println("\npublishing...\n");

      // always have to convert from int to char array
      char mybytes[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      unsigned int myFree = ESP.getFreeHeap();
      //      char mybytes2[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      sprintf(mybytes, "%u" , myFree);
      client.publish("sm1/memfree", mybytes);

      //      char mybytes[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0}   ;
      sprintf(mybytes, "%u" , myMins);
      //      mybytes[5] = 0x0;
      client.publish("sm1/uptime", mybytes);

      delay(100);

      sprintf(mybytes, "%u" , inttempF);
      client.publish("sm1/temp", mybytes);
      delay(100);

      sprintf(mybytes, "%u" , capread);
      client.publish("sm1/moisture", mybytes);
      delay(100);

      client.publish("sm1/version", VERSION);
      digitalWrite(myLed, HIGH);

    }
    //    oldMillis = myMillis;
  }

//  client.loop();
  if (SleepStatus == 1 && millis() > 20000) {   // was 70000

    // turn off the seesaw
    digitalWrite(D6, LOW);

    client.publish("sm1/status", "sleeping");
    client.loop();
    Serial.println("\n sleeping...\n");
    delay(5000);
    WiFi.disconnect();
    delay(2000);
    ESP.deepSleep(300e6);
  }

}

//*************************************************
void setupWifi() {

  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration

  IPAddress staticIP(192, 168, 2, MYADDR); //ESP static ip
  IPAddress dns(192, 168, 2, 6);           //DNS
  IPAddress gateway(192, 168, 2, 1);       //IP Address of your WiFi Router (Gateway)
  IPAddress subnet(255, 255, 255, 0);      //Subnet mask

  WiFi.mode(WIFI_STA);
  WiFi.hostname(WiFi_hostname);

  WiFi.config(staticIP, dns, gateway, subnet);
  WiFi.begin(SSID, SSIDPWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(WiFi_hostname)) {
    Serial.println(F("MDNS responder started"));
    MDNS.addService("http", "tcp", 80);
    //    MDNS.addServiceTxt("arduino","tcp","supports",supportedHost);
  }

}

//*************************************************
void callback(char* topic, byte* payload, unsigned int length) {

  for (int ii = 0; ii < length; ii++) {
    payload[ii] = toupper(payload[ii]);
  }
  payload[length] = '\0';
  char *chrPayload = (char *) payload;

  if (strcmp(topic, "sm1/modereq") == 0)
  {
    if (strncmp(chrPayload, "AWAKE", 5) == 0) {
      client.publish("sm1/status", "awake");
      SleepStatus = 0;
    }
    if (strncmp(chrPayload, "SLEEP", 5) == 0) {
      client.publish("sm1/status", "sleep pending");
      delay(5000);
      SleepStatus = 1;
    }
    if (strncmp(chrPayload, "REBOOT", 6) == 0) {
      client.publish("sm1/status", "Rebooting");
      ESP.restart();
    }
  }
}  // end callback


//*************************************************
void setupOTA() {

  ArduinoOTA.setHostname(WiFi_hostname);
  ArduinoOTA.onStart([]() {
    digitalWrite(warning, HIGH);
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite(warning, !digitalRead(warning));
  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end

    for (int i = 0; i < 10; i++)
    {
      digitalWrite(myLed, HIGH);
      digitalWrite(warning, HIGH);
      delay(100);
      digitalWrite(myLed, LOW);
      digitalWrite(warning, LOW);
      delay(100);
    }

    digitalWrite(myLed, LOW);   // turn off blue LED

    // had to add on this computer to make OTA finish properly
    ESP.restart();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });

  // setup the OTA server
  ArduinoOTA.begin();

}

//********************************
void reconnect() {
  int attempts = 0;
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_client, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      cntMR += 1;
      client.publish("sm1/status", "up" );
      client.subscribe("sm1/#");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // blink twice
      for (int i = 0 ; i < 2; i++) {
        digitalWrite(myLed, LOW);
        delay(500);
        digitalWrite(myLed, HIGH);
        delay(500);
      }
      attempts += 1;
      // Wait 5 seconds before retrying
      delay(5000);
      if (attempts > 120) {
        ESP.restart();
      }

    }
  }
} // end reconnect
