#include <Arduino.h>
#include "WiFi.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

//wifi autka i haslo
const char* ssid = "ESP_CAR";
const char* password = "123456789";

// Zdefiniowanie silniczkow A i B
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

int motor2Pin1 = 23; 
int motor2Pin2 = 22; 
int enable2Pin = 12;  

String MotorState;

String pwmSliderValue = "0";

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
//int car_speed = 255;

//wartosci do zmiany predkosci autka
const char* PARAM_INPUT = "value";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with MOTOR state value
String processor(const String& var){
  Serial.println(var);
  if (var == "SLIDERVALUE"){
    return pwmSliderValue;
  }
  return String();
}


void setup() {

  Serial.begin(115200);

  //ustawiam ze piny sa wyjsciowe do sterowania silnikami
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  // konfiguracja PWM
  ledcSetup(pwmChannel, freq, resolution);

  // attach the channel to the GPIOs to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel);

  //car_speed = sliderValue.toInt();
  ledcWrite(pwmChannel, pwmSliderValue.toInt());

  //ustawiam kazdy pin na 0 bo domyslnie autko ma stac w miejscu
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW); 
  digitalWrite(motor2Pin2, LOW);

  // Inicjalizacja biblioteki SPIFFS
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

//-------------------------------------------------------------------------
    // STOP MOTOR
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, pwmSliderValue.toInt());
    //delay(2000);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // FORWARD
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    ledcWrite(pwmChannel, pwmSliderValue.toInt());
    //delay(2000);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // BACKWARD
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, HIGH); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, pwmSliderValue.toInt());
    //delay(2000);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

   // LEFT
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    ledcWrite(pwmChannel, pwmSliderValue.toInt());

    //delay(2000);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

   // RIGHT
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, pwmSliderValue.toInt());
    //delay(2000);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      pwmSliderValue = inputMessage;
      //car_speed = sliderValue.toInt();
      ledcWrite(pwmChannel, pwmSliderValue.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(SPIFFS, "text/plain", "OK");
  });
 
  // Start server
  server.begin();

}

void loop() {
 


}
