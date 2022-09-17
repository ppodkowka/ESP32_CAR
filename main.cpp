#include <Arduino.h>
#include "WiFi.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const char* ssid = "ESP_CAR";
const char* password = "123456789";

// Zdefiniowanie Motora A i B
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;
int motor2Pin1 = 23; 
int motor2Pin2 = 22; 
int enable2Pin = 12;  

String carSpeed = "0"; // duty cycle
const char* PARAM_INPUT = "value";

// Setting PWM properties
const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

//dzieki tej funkcji podmieniam SLIDERVALUE na faktyczna liczbe
String processor(const String& var){
  Serial.println(var);
  if (var == "CARSPEED"){
    return carSpeed;
  }
  return String();
}

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void setup() {

  Serial.begin(115200);

  //inicjalizacja motorów
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);

  // attach the channel to the GPIOs to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel);

  ledcWrite(pwmChannel, carSpeed.toInt());

  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW); 
  digitalWrite(motor2Pin2, LOW);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //robie z esp wlasna siec wifi
  Serial.print("Setting AP (Access Point)…\n");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  //root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  //load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  //load scripts.js file
  server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/scripts.js", "text/javascript");
  });

//-------------------------------------------------------------------------

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
  String inputMessage;
  // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
  if (request->hasParam(PARAM_INPUT)) {
    inputMessage = request->getParam(PARAM_INPUT)->value();
    carSpeed = inputMessage;
    ledcWrite(pwmChannel, carSpeed.toInt());
  }
  else {
    inputMessage = "No message sent";
  }
  Serial.println(inputMessage);
  //request->send(200, "text/plain", "OK");
  request->send(SPIFFS, "/index.html");
  });

    // STOP MOTOR
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    //delay(2000);
    ledcWrite(pwmChannel, carSpeed.toInt());    
    request->send(SPIFFS, "/index.html");
  });
  
  // FORWARD
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    //delay(2000);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

  // BACKWARD
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, HIGH); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH); 
    digitalWrite(motor2Pin2, LOW);
    //delay(2000);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

   // LEFT
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    //delay(2000);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

   // RIGHT
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    //delay(2000);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });
 

  // Start server
  server.begin();


}

void loop() {
 


}
