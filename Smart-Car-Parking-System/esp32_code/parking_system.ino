#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "KJAH_503";
const char* password = "ece210912132229";

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo servo;

unsigned long startTime1 = 0, startTime2 = 0;
bool active1 = false, active2 = false;
const int ratePerSecond = 2;

bool isCarInFront = false; 
bool isGateOpen = false;
bool exitSensorTriggered = false;

void handleStatus() {
  server.sendHeader("Access-Control-Allow-Origin", "*");

  String json = "{ \"slot1\":" + String(digitalRead(4) == 0 ? "true" : "false") + ",";
  json += "\"slot2\":" + String(digitalRead(16) == 0 ? "true" : "false") + ",";
  json += "\"s1_duration\":" + String(active1 ? (millis() - startTime1)/1000 : 0) + ",";
  json += "\"s2_duration\":" + String(active2 ? (millis() - startTime2)/1000 : 0) + "}";

  server.send(200, "application/json", json);
}

void handleCarLogic() {
  if (server.hasArg("state")) {
    String state = server.arg("state");

    // 🔥 DEBUG VISIBILITY
    Serial.print("Received state: ");
    Serial.println(state);

    if (state == "1") {
      isCarInFront = true;
      Serial.println("AI SIGNAL: 1");
    } else {
      isCarInFront = false;
      Serial.println("AI SIGNAL: 0");
    }
  } else {
    Serial.println("No state argument received");
  }

  server.send(200, "text/plain", "OK");
}

void setup(){
  pinMode(16, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/status", handleStatus);
  server.on("/car_logic", handleCarLogic);
  server.begin();

  servo.attach(14);
  servo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void loop (){
  server.handleClient(); 

 // --- GATE LOGIC ---
bool slotAvailable = (digitalRead(4) == 1  digitalRead(16) == 1);
bool entryTriggered = (digitalRead(2) == 0  digitalRead(15) == 0);

if (slotAvailable && isCarInFront && entryTriggered) {
  servo.write(90);
  isGateOpen = true;
  Serial.println("GATE OPEN: Car Entering");
}

// CLOSE CONDITION (THIS IS WHAT YOU WERE MISSING)
if (isGateOpen && !entryTriggered) {
  servo.write(0);
  isGateOpen = false;
  Serial.println("GATE CLOSED");
}

  // --- BILLING ---
  static unsigned long s1_timer = 0, s2_timer = 0;

  lcd.setCursor(0, 0);
  if (digitalRead(4) == 0) { 
    if (!active1) { startTime1 = millis(); active1 = true; }
    lcd.print("S1: Occupied    ");
  } else {
    if (active1) {
      lcd.print("S1 Bill: ");
      lcd.print(((millis()-startTime1)/1000)*ratePerSecond);
      lcd.print(" TK   ");
      active1 = false;
      s1_timer = millis();
    } else if (millis() - s1_timer > 2000) {
      lcd.print("S1: Empty       ");
    }
  }

  lcd.setCursor(0, 1);
  if (digitalRead(16) == 0) { 
    if (!active2) { startTime2 = millis(); active2 = true; }
    lcd.print("S2: Occupied    ");
  } else {
    if (active2) {
      lcd.print("S2 Bill: ");
      lcd.print(((millis()-startTime2)/1000)*ratePerSecond);
      lcd.print(" TK   ");
      active2 = false;
      s2_timer = millis();
    } else if (millis() - s2_timer > 2000) {
      lcd.print("S2: Empty       ");
    }
  }
}