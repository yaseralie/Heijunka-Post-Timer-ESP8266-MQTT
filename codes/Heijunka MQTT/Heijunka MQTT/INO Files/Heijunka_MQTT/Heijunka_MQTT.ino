//Using ESP8266
//This program used for turn on heijunka light at specified time
//The time get from mqtt messages
//Time shown in 7 segment
//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

//Setup for MQTT and WiFi==================================
#include <ESP8266WiFi.h>
//Library for MQTT:
#include <PubSubClient.h>
//Edit: MQTT_MAX_PACKET_SIZE 1040 in PubSubClient.h

//Library for Json format using version 5:
#include <ArduinoJson.h>
//Setup for MQTT and WiFi==================================

//Setup for Real Time Clock DS3231=========================
#include <Wire.h>
#include <ds3231.h>
//Setup for Real Time Clock DS3231=========================

//Setup for Max7219======================================
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
struct ts t;
//Setup for LED Max7219==============================
// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15
// Hardware SPI connection
MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

//declare MQTT Topic=======================================
const char* topic_pub = "heijunka1_from_esp";
//declare topic for subscribe message
const char* topic_sub = "heijunka1_to_esp";
//declare MQTT Topic=======================================

// Update these with values suitable for your network======
const char* ssid = "YOUR SSID WIFI";
const char* password = "YOUR PASSWORD";
const char* mqtt_server = "100.123.456.100"; //FIll This your broker IP Address
// Update these with values suitable for your network======

//for input - output=======================================
const int BUTTON_PIN = 0; // D3
const int RELAY_PIN  = 2; // D4

//variables================================================
String status_text = "clock";
String time_now = "00:00";
String time_hour = "0";
String time_min = "0";
int i = 0;
String mqtt_status = "standby";
String notif = "";
const char* notif_pick ;

//Set Default of Schedule of timer====================================
//Example: "8:0:0,9:0:0,10:0:0"
String schedule = "5:0:0,5:10:0,5:20:0,5:30:0,5:40:0,5:50:0,6:0:0,6:10:0,6:20:0,6:30:0,6:40:0,6:50:0,7:0:0,7:10:0,7:20:0,7:30:0,7:40:0,7:50:0,8:0:0,8:10:0,8:20:0,8:30:0,8:40:0,8:50:0,9:0:0,9:10:0,9:20:0,9:30:0,9:40:0,9:50:0,10:0:0,10:10:0,10:20:0,10:30:0,10:40:0,10:50:0,11:0:0,11:10:0,11:20:0,11:30:0,11:40:0,11:50:0,12:0:0,12:10:0,12:20:0,12:30:0,12:40:0,12:50:0,13:0:0,13:10:0,13:20:0,13:30:0,13:40:0,13:50:0,14:0:0,14:10:0,14:20:0,14:30:0,14:40:0,14:50:0,15:0:0,15:10:0,15:20:0,15:30:0,15:40:0,15:50:0,16:0:0,16:10:0,16:20:0,16:30:0,16:40:0,16:50:0,17:0:0,17:10:0,17:20:0,17:30:0,17:40:0,17:50:0,18:0:0,18:10:0,18:20:0,18:30:0,18:40:0,18:50:0,19:0:0,19:10:0,19:20:0,19:30:0,19:40:0,19:50:0,20:0:0,20:10:0,20:20:0,20:30:0,20:40:0,20:50:0,21:0:0,21:10:0,21:20:0,21:30:0,21:40:0,21:50:0,22:0:0,22:10:0,22:20:0,22:30:0,22:40:0,22:50:0,23:0:0,23:10:0,23:20:0,23:30:0,23:40:0,23:50:0,0:0:0,0:10:0,0:20:0,0:30:0,0:40:0,0:50:0,1:0:0,1:10:0,1:20:0,1:30:0,1:40:0,1:50:0,2:0:0,2:10:0,2:20:0,2:30:0,2:40:0,2:50:0,3:0:0,3:10:0,3:20:0,3:30:0,3:40:0,3:50:0,4:0:0,4:10:0,4:20:0,4:30:0,4:40:0,4:50:0";

WiFiClient espClient;
PubSubClient client(espClient);

//SETUP WIFI******************************************************
void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
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
//SETUP WIFI******************************************************

//FUNCTION TO RECEIVE MQTT MESSAGE********************************
void callback(char* topic, byte* payload, unsigned int length)
{
  //Receiving message as subscriber
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  String json_received;

  Serial.print("JSON Received:");
  for (int i = 0; i < length; i++) {
    json_received += ((char)payload[i]);
    //Serial.print((char)payload[i]);
  }
  Serial.println(json_received);
  if (json_received == "set_time")
  {
    mqtt_status = "set_time";
    status_text = "wait";
    Serial.println("Waiting Next message to set time");
  }
  else if (json_received == "set_schedule")
  {
    mqtt_status = "set_schedule";
    status_text = "wait";
    Serial.println("Waiting Next message to set schedule");
  }
  else
  {
    if (mqtt_status == "set_schedule")
    {
      schedule = json_received;
      status_text = "new_schedule";
    }

    else if (mqtt_status == "set_time")
    {
      //Parse json
      //StaticJsonBuffer<1040> jsonBuffer;
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(json_received);

      //get json parsed value
      //sample of json: {"hour":8,"minute":0,"second":0,"day":1,"month":1,"year":2022}
      String h = root["hour"];
      String m = root["minute"];
      String s = root["second"];
      String d = root["day"];
      String mon = root["month"];
      String y = root["year"];
            
      Serial.println(y);
      Serial.println(mon);
      Serial.println(d);
      Serial.println(h);
      Serial.println(m);
      Serial.println(s);
      Serial.println("-----------------------");

      //Set current time based on receive configuration time
      t.hour = h.toInt();
      t.min = m.toInt();
      t.sec = s.toInt();
      t.mday = d.toInt();
      t.mon = mon.toInt();
      t.year = y.toInt();

      DS3231_set(t);
      //set status to show in display
      status_text = "new_time";
    }
    mqtt_status = "standby";
  }
}
//FUNCTION TO RECEIVE MQTT MESSAGE********************************

//FUNCTION TO RECONNECT MQTT**************************************
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if you MQTT broker has clientID,username and password
    //please change following line to    if (client.connect(clientId,userName,passWord))
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      //once connected to MQTT broker, subscribe command if any
      client.subscribe(topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//FUNCTION TO RECONNECT MQTT**************************************

//FUNCTION SETUP@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void setup() {
  //setup FOR MAX7219-------------------------------------------
  Display.begin();
  Display.setIntensity(8);
  Display.displayClear();

  //setup serial and RTC------------------------------------
  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);

  //setup wifi and mqtt-------------------------------------------
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //subscribe topic
  client.subscribe(topic_sub);

  //setup pin output----------------------------------------------
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  //test relay relay-------------------------------------------------
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
}
//FUNCTION SETUP@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //for RTC======================================================
  DS3231_get(&t);

  if (t.hour < 10)
  {
    time_hour = "0" + String(t.hour);
  }
  else
  {
    time_hour = String(t.hour);
  }

  if (t.min < 10)
  {
    time_min = "0" + String(t.min);
  }
  else
  {
    time_min = String(t.min);
  }

  if ( (t.sec % 2) == 0) {
    // Display the current time in 24 hour format with leading zeros enabled and a center colon:
    time_now = time_hour + ":" +  time_min ;
  }
  else {
    time_now = time_hour + " " +  time_min ;
  }
  
  //get now time
  String time_in_display = String(t.hour) + ":" +  String(t.min) + ":" + String(t.sec) ;

  //find time_in_display in schedule----------------------------------------------
  int post = schedule.indexOf(time_in_display);
  if (post >= 0) {
    Serial.println("Relay ON");
    status_text = "pick_time";
    digitalWrite(RELAY_PIN, LOW); // turn on
    //Set notification for timer
    notif = "Time to pick up kanban: " + String(time_now);
    notif_pick = notif.c_str();
  }

  //Check button to turn off relay-------------------------------------------------
  int buttonState = digitalRead(BUTTON_PIN); // read new state
  if (buttonState == LOW) {
    Serial.println("The button pressed");
    digitalWrite(RELAY_PIN, HIGH); // turn off
    status_text = "clock";
  }

  //Display text to Dot matrix===================================
  if (status_text == "new_time")
  {
    if (Display.displayAnimate())
      //Set running text
      Display.displayText("Received New Time Configuration...", PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
  else if (status_text == "new_schedule")
  {
    if (Display.displayAnimate())
      //Set running text
      Display.displayText("Received New Time Schedule...", PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
  else if (status_text == "pick_time")
  {
    //Set running text
    if (Display.displayAnimate())
      Display.displayText(notif_pick, PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }
  else if (status_text == "clock")
  {
    Display.setTextAlignment(PA_CENTER);
    Display.print(time_now.c_str());
    i = 0;
    delay(500);
  }
  else if (status_text == "wait")
  {
    Display.setTextAlignment(PA_LEFT);
    Display.print("Wait...");
  }
}
