//Using ESP8266
//This program used for turn on heijunka light at specified time
//Time shown in 7 segment
//Reference GPIO  https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

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

//for input - output=======================================
const int BUTTON_PIN = 0; // D3
const int RELAY_PIN  = 2; // D4

//variables================================================
String status_text = "clock";
String time_now = "00:00";
String time_hour = "0";
String time_min = "0";
int i = 0;
String notif = "";
const char* notif_pick ;

//Set Schedule of timer====================================
//Example: "8:0:0,9:0:0,10:0:0"
String schedule = "20:30:0,20:32:0,20:34:0,20:36:0,20:38:0,20:40:0,20:42:0,20:44:0,20:46:0,20:48:0,20:50:0,20:52:0,20:54:0,20:56:0,20:58:0,21:0:0,21:2:0,21:4:0,21:6:0,21:8:0,21:10:0,21:12:0,21:14:0,21:16:0,21:18:0,21:20:0,21:22:0,21:24:0,21:26:0,21:28:0,21:30:0,21:32:0,21:34:0,21:36:0,21:38:0,21:40:0,21:42:0,21:44:0,21:46:0,21:48:0,21:50:0,21:52:0,21:54:0,21:56:0,21:58:0,22:0:0,22:2:0,22:4:0,22:6:0,22:8:0,22:10:0,22:12:0,22:14:0,22:16:0,22:18:0,22:20:0,22:22:0,22:24:0,22:26:0,22:28:0,22:30:0,22:32:0,22:34:0,22:36:0,22:38:0,22:40:0,22:42:0,22:44:0,22:46:0,22:48:0,22:50:0,22:52:0,22:54:0,22:56:0,22:58:0,23:0:0,23:2:0,23:4:0,23:6:0,23:8:0,23:10:0,23:12:0,23:14:0,23:16:0,23:18:0,23:20:0,23:22:0,23:24:0,23:26:0,23:28:0,23:30:0,23:32:0,23:34:0,23:36:0,23:38:0,23:40:0,23:42:0,23:44:0,23:46:0,23:48:0,23:50:0,23:52:0,23:54:0,23:56:0,23:58:0,0:0:0,0:2:0,0:4:0,0:6:0,0:8:0,0:10:0,0:12:0,0:14:0,0:16:0,0:18:0,0:20:0,0:22:0,0:24:0,0:26:0,0:28:0";

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

  //find time_in_display in schedule
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
    Serial.println("The button is being pressed");
    digitalWrite(RELAY_PIN, HIGH); // turn off
    status_text = "clock";
  }

  //Display text to Dot matrix===================================
  if (status_text == "new_config")
  {
    if (Display.displayAnimate())
      //Set running text
      Display.displayText("Received New Time Configuration...", PA_LEFT, 100, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
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
}
