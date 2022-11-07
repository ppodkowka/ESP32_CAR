#include <Arduino.h>
#include "WiFi.h"
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

// setting car wifi credentials
const char* ssid = "ESP_CAR";
const char* password = "123456789";

// defining motor 1 and 2
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;
int motor2Pin1 = 23; 
int motor2Pin2 = 22; 
int enable2Pin = 12;  

String carSpeed = "0"; // this variable holds value of slider which changes speed of car
const char* PARAM_INPUT = "value"; // this variable is used to search for slider value on the request received when slider is moved

// setting PWM properties
const int freq = 500;
const int pwmChannel = 0;
const int resolution = 8;

// this function replaces "CARSPEED" with current value of slider
String processor(const String& var){
  Serial.println(var);
  if (var == "CARSPEED"){
    return carSpeed;
  }
  return String();
}

// creating AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup() {

  Serial.begin(115200);

  // initialization of motors
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);
  pinMode(enable2Pin, OUTPUT);

  // configurating PWM
  ledcSetup(pwmChannel, freq, resolution);

  // attach the channel to the GPIOs 
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel);

  // setting dutycycle of PWM
  ledcWrite(pwmChannel, carSpeed.toInt());

  // setting all pins of motor 1 and 2 to low at the beggining
  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW); 
  digitalWrite(motor2Pin2, LOW);

  // initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // setting ESP as access point
  Serial.print("Setting AP (Access Point)…\n");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // load index.html file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // load scripts.js file
  server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/scripts.js", "text/javascript");
  });

  // this section gets value of slider and then updates PWM duty cycle (car speed)
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      carSpeed = inputMessage;
      ledcWrite(pwmChannel, carSpeed.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(SPIFFS, "/index.html");
  });

    // STOP the car
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, carSpeed.toInt());    
    request->send(SPIFFS, "/index.html");
  });
  
  // go FORWARD
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

  // go BACKWARD
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, HIGH); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

   // turn LEFT
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });

   // turn RIGHT
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    ledcWrite(pwmChannel, carSpeed.toInt());
    request->send(SPIFFS, "/index.html");
  });
 
  // start the server
  server.begin();


}

void loop() {
 

}
