#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const char* ssid = "CybernetWMW2598";
const char* password = "2503AD2020";

// Motor A
int motor1Pin1 = 27; 
int motor1Pin2 = 26; 
int enable1Pin = 14;

// Motor B
int motor2Pin1 = 23; 
int motor2Pin2 = 22; 
int enable2Pin = 12;  

String MotorState;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 250;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with MOTOR state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(motor1Pin1) == LOW && digitalRead(motor1Pin2) == LOW && digitalRead(motor2Pin1) == LOW && digitalRead(motor2Pin2) == LOW){
      MotorState = "STOP";
    }
    else if(digitalRead(motor1Pin1) == LOW && digitalRead(motor1Pin2) == HIGH && digitalRead(motor2Pin1) == LOW && digitalRead(motor2Pin2) == HIGH){
      MotorState = "FORWARD";
    }
    else if(digitalRead(motor1Pin1) == HIGH && digitalRead(motor1Pin2) == LOW && digitalRead(motor2Pin1) == HIGH && digitalRead(motor2Pin2) == LOW){
      MotorState = "BACKWARD";
    }
    else if(digitalRead(motor1Pin1) == LOW && digitalRead(motor1Pin2) == LOW && digitalRead(motor2Pin1) == LOW && digitalRead(motor2Pin2) == HIGH){
      MotorState = "LEFT";
    }
    else if(digitalRead(motor1Pin1) == HIGH && digitalRead(motor1Pin2) == HIGH && digitalRead(motor2Pin1) == LOW && digitalRead(motor2Pin2) == LOW){
      MotorState = "RIGHT";
    }
    Serial.print(MotorState);
    return MotorState;
  }
  return String();
}



void setup() {

  Serial.begin(115200);
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

  digitalWrite(motor1Pin1, LOW); 
  digitalWrite(motor1Pin2, LOW);
  digitalWrite(motor2Pin1, LOW); 
  digitalWrite(motor2Pin2, LOW);

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("\nConnecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

//-------------------------------------------------------------------------
    // STOP MOTOR
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    dutyCycle = 255;
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    delay(2000);
    ledcWrite(pwmChannel, dutyCycle);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // FORWARD
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
    dutyCycle = 255;
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    delay(2000);
    ledcWrite(pwmChannel, dutyCycle);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // BACKWARD
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
    dutyCycle = 255;
    digitalWrite(motor1Pin1, HIGH); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, HIGH); 
    digitalWrite(motor2Pin2, LOW);
    delay(2000);
    ledcWrite(pwmChannel, dutyCycle);      
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

   // LEFT
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    dutyCycle = 255;
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, HIGH);
    delay(2000);
    ledcWrite(pwmChannel, dutyCycle);      
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

   // RIGHT
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    dutyCycle = 255;
    digitalWrite(motor1Pin1, LOW); 
    digitalWrite(motor1Pin2, HIGH);
    digitalWrite(motor2Pin1, LOW); 
    digitalWrite(motor2Pin2, LOW);
    delay(2000);
    ledcWrite(pwmChannel, dutyCycle);      
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
 

  // Start server
  server.begin();


}

void loop() {
 


}
