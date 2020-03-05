///// Create date 27.10.2019 ///////
///// v1.0 /////////////////////////



///////////////////////////// For add Library /////////////////////////////////

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;


///////////////////////////// End add Library /////////////////////////////////



///////////////////////////// For Declar Valable ///////////////////////////////

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//WebServer
ESP8266WebServer server(80);

const char *ssid1 = "Smart Home Control"; // The name of the Wi-Fi network that will be created
const char *password1 = "12345678";       // The password required to connect to it, leave blank for an open network

// WiFi Parameters
const char* ssid2 = "Virus";
const char* password2 = "R@dioactive";

//shift register
int dataPin = 2;       //Pin connected to DS of 74HC595   -> 14, esp's D4
int latchPin = 14;      //Pin connected to ST_CP of 74HC595 -> 12, esp's D5
int clockPin = 12;      //Pin connected to SH_CP of 74HC595 -> 11, esp's D6

int ON_STATE = 1;
int OFF_STATE = 0;
byte controldata_SR = 0b00000000;

///////////////////////////// End Declar Valable ///////////////////////////////



///////////////////////////// For Setup Function ///////////////////////////////

void setup() {
  Serial.begin(115200);

  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  relay_out_1to8(0,OFF_STATE); // all pin out off
  
  delay(10);

  Wire.begin();
  RTC.begin();

  //myHotspotDisconnect();
  accessWiFi(); // WiFi access

  server.on("/", []() {
    server.send(200, "text/html", " <h1> Wecome! from Smart Home Control </h1>");
  });
  server.on("/switch8Channel", switch8Channel);
  server.on("/currentSwitchStatus", currentSwitchStatus);
  server.on("/switchAllOnOff", switchAllOnOff);
  server.on("/currentDateAndTime", currentDateAndTime);
  server.on("/clockTimerSwitch", clockTimerSwitch);
  server.begin();

  //RTC
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //RTC.adjust(DateTime(__DATE__, __TIME__));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    RTC.adjust(DateTime(2019, 10, 6, 22, 57, 0));
  }
  //RTC.adjust(DateTime(2019, 10, 6, 23, 4, 0));

}

///////////////////////////// End Setup Function ///////////////////////////////




///////////////////////////// For Loop Function ///////////////////////////////

void loop() {
  server.handleClient();
}

///////////////////////////// End Loop Function ///////////////////////////////




//////////////////// For WiFi ////////////////////////////////////////////////

void myHotspot() {
  Serial.println('\n');
  WiFi.softAP(ssid1, password1);             // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid1);
  Serial.println("\" started");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
}

void myHotspotDisconnect(){
  WiFi.softAPdisconnect(true);
}

void accessWiFi() {
  WiFi.begin(ssid2, password2);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.print("Connected to ");
  Serial.println(ssid2);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//////////////////////////////// End WiFi /////////////////////////////////




///////////////////////////// For Switch 8 channel ////////////////////////

void switch8Channel() {
  String data = server.arg("plain");
  Serial.println(data);

  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  String switchType = jObject["switchType"];  // 1 to 8
  String action     = jObject["action"];    // on , off
  action.toLowerCase();

  Serial.println();
  Serial.print("SwitchTpye: "); Serial.print(switchType);
  Serial.println();
  Serial.print("Action: "); Serial.print(action);
  Serial.println();

  if (action == "on" && switchType.toInt() > 0 && switchType.toInt() < 9)
  {
    relay_out_1to8(switchType.toInt() - 1, ON_STATE);
    server.send(200, "application/json", "{\"msg\":1}");  // successfully
    Serial.println("Successfully");
  }
  else if (action == "off" && switchType.toInt() > 0 && switchType.toInt() < 9)
  {
    relay_out_1to8(switchType.toInt() - 1, OFF_STATE);
    server.send(200, "application/json", "{\"msg\":1}"); // successfully
    Serial.println("Successfully");
  }
  else {
    server.send(200, "application/json", "{\"msg\":0}"); // no successfully
    Serial.println("No Successfully");
  }

}

void currentSwitchStatus(){
  String switchData = String(controldata_SR, BIN);
  String addData = "";

  for (int i = 0; i < 8 - switchData.length(); i++)
  {
    addData = "0" + addData;
  }

  switchData = addData + switchData;

  server.send(200, "application/json", "{ \"switch1\":" + String(switchData.charAt(7)) + ", \"switch2\":"+ String(switchData.charAt(6)) + 
                                        ", \"switch3\":" + String(switchData.charAt(5)) + ", \"switch4\":" + String(switchData.charAt(4)) +
                                        ", \"switch5\":" + String(switchData.charAt(3)) + ", \"switch6\":" + String(switchData.charAt(2)) + 
                                        ", \"switch7\":" + String(switchData.charAt(1)) + ", \"switch8\":" + String(switchData.charAt(0)) + " }");
}

void switchAllOnOff(){
  String data = server.arg("plain");
  Serial.println(data);

  StaticJsonBuffer<200> jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  String switchAll = jObject["switchAll"];  // on , off
  switchAll.toLowerCase();

  if (switchAll == "on")
  {
    controldata_SR = 0b11111111;
    relay_out_1to8(0, 1);
    server.send(200, "application/json", "{\"msg\":1}"); // successfully
    Serial.println("Successfully");
  }
  else if (switchAll == "off")
  {
    controldata_SR = 0b00000000;
    relay_out_1to8(0, 0);
    server.send(200, "application/json", "{\"msg\":1}"); // successfully
    Serial.println("Successfully");
  }
  else{
    server.send(200, "application/json", "{\"msg\":0}"); // no successfully
    Serial.println("No Successfully");
  }

}

///////////////////////////////// End Switch 8 channel ///////////////////////////




//////////////////////////////// For Date and Time ////////////////////////////////

void currentDateAndTime() {
  DateTime now = RTC.now();

  //  String currentDT = " {\"currYear\":\"" + String(now.year()) + "\", " + "\"currMonth\":\"" + String(now.month()) + "\", " + "\"currDay\":\"" + String(now.day()) + "\", " +
  //                     "\"currHour\":\"" + String(now.hour()) + "\", " + "\"currMinute\":\"" + String(now.minute()) + "\", " + "\"currSecond\":\"" + String(now.second()) + "\", " +
  //                     "\"currDayOfWeek\":\"" + String(daysOfTheWeek[now.dayOfTheWeek()]) + "\" }";

  String currentDT = " {\"currYear\":" + String(now.year()) + ", " + "\"currMonth\":" + String(now.month()) + ", " + "\"currDay\":" + String(now.day()) + ", " +
                     "\"currHour\":" + String(now.hour()) + ", " + "\"currMinute\":" + String(now.minute()) + ", " + "\"currSecond\":" + String(now.second()) + ", " +
                     "\"currDayOfWeek\":\"" + String(daysOfTheWeek[now.dayOfTheWeek()]) + "\" }";

  //Serial.println(currentDT);

  server.send(200, "application/json", currentDT );

}

void clockTimerSwitch() {

}

/////////////////////////////// End Date and Time ////////////////////////////////




////////////////////////////// For Other Function ///////////////////////////////////

void relay_out_1to8(int index, int action_) {

  bitWrite(controldata_SR, index, action_); // 0000 0000, 1, 1 ==> 0000 0010 , index is starting last number (0 to 7)

  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, controldata_SR);
  digitalWrite(latchPin, HIGH);
}

/////////////////////////////// End Other Function //////////////////////////////////
