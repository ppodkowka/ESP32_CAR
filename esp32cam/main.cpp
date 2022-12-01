#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>
#include "SPIFFS.h"
#include "Adafruit_INA219.h"
#include <Wire.h>
#include <HardwareSerial.h>


//Camera related constants
#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid     = "MyCar";
const char* password = "123456789";

AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
uint32_t cameraClientId = 0;


void onCameraWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0;
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setupCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }  

  if (psramFound())
  {
    heap_caps_malloc_extmem_enable(20000);  
    Serial.printf("PSRAM initialized. malloc to take memory from psram above this size");    
  }  
}

void sendCameraPicture(){
  if (cameraClientId == 0)
  {
    return;
  }
  unsigned long  startTime1 = millis();
  //capture a frame
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) 
  {
      Serial.println("Frame buffer could not be acquired");
      return;
  }

  unsigned long  startTime2 = millis();
  wsCamera.binary(cameraClientId, fb->buf, fb->len);
  esp_camera_fb_return(fb);
    
  //Wait for message to be delivered
  while (true)
  {
    AsyncWebSocketClient * clientPointer = wsCamera.client(cameraClientId);
    if (!clientPointer || !(clientPointer->queueIsFull()))
    {
      break;
    }
    delay(1);
  }
}


//                                   ------- CAR CONTROLL SECTION ------

// defines for UART communication 
HardwareSerial mySerial(2);
const int myRX = 15;
const int myTX = 14;
char command  = ' ';

// for battery tester
String battery_percent  = "--%";

// defining enable pin for motor 1 and 2
 
const int enablePin = 2;  

int stopFlag = 0;
String direction;

// defines for controlling LED 
const int ledPin = 4;
int ledState = 0;   
int ledBrightness = 0;
int ledFlag = 0;

//Setting PWM properties
const int freq = 1000;
const int pwmChannel = 0;
const int pwmChannelLED = 1;
const int pwmResolution = 8;
int speed = 250;


void setCar(){
  // setting motor enable pin to output and ledPin
  pinMode(enablePin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // configurating PWM
  ledcSetup(pwmChannel, freq, pwmResolution);
  ledcSetup(pwmChannelLED, freq, pwmResolution);

  // attach the channel to the GPIO 
  ledcAttachPin(enablePin, pwmChannel);
  ledcAttachPin(ledPin, pwmChannelLED);

  // setting speed of PWM
  ledcWrite(pwmChannel, speed);
  ledcWrite(pwmChannelLED, ledBrightness);

  // setting ledPin to low at the beggining
  digitalWrite(ledPin, LOW);
}


// this function replaces "BATTERY" with current value of slider
String processor(const String& var){
  //Serial.println(var);
  if (var == "BATTERY"){
    return battery_percent;
  }
  return String();
}


void setup(void) 
{
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, myRX, myTX);

  setCar();
  setupCamera();

  // initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

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
      
  wsCamera.onEvent(onCameraWebSocketEvent);
  server.addHandler(&wsCamera);

  // STOP the car
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("STOP"); 
    mySerial.print(0); // tell slave to stop car
    request->send(SPIFFS, "/index.html");
  });

  // go FORWARD
  server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("FORWARD");
    direction = "forward";
    if(stopFlag == 0){
      mySerial.print(1); // tell slave to go forward  
    }
    request->send(SPIFFS, "/index.html");
  });

  // go BACKWARD
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("BACKWARD");
    direction = "backward";
    mySerial.print(2); // tell slave to go backwards 
    stopFlag = 0;
    request->send(SPIFFS, "/index.html");
  });

  // turn LEFT
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("LEFT");
    direction = "left";
    mySerial.print(4); // tell slave to go left 
    stopFlag = 0;
    request->send(SPIFFS, "/index.html");
  });

  // turn RIGHT
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("RIGHT");
    direction = "right";
    mySerial.print(3); // tell slave to go right
    stopFlag = 0;
    request->send(SPIFFS, "/index.html");
  });

  // speed slider
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("slider");
    String inputMessage;
    if (request->hasParam("value")) {
      inputMessage = request->getParam("value")->value();
      speed = inputMessage.toInt();
      ledcWrite(pwmChannel, speed);
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(SPIFFS, "/index.html");
  });

  // LED
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("LED");
    if(ledFlag == 0){
      ledFlag = 1;
      ledBrightness = 250;
      ledcWrite(pwmChannelLED, ledBrightness);
      digitalWrite(ledPin, HIGH);
    }
    else if(ledFlag == 1){
      ledFlag = 0;
      ledBrightness = 0;
      ledcWrite(pwmChannelLED, ledBrightness);
      digitalWrite(ledPin, LOW);
    }
    request->send(SPIFFS, "/index.html");
  });

  server.begin(); 
}


void loop() 
{

  if (mySerial.available())
  {
    char command = mySerial.read();
   
    if (command == '1') {
      battery_percent = "10%";
    }
    else if (command == '2') {
      battery_percent = "20%";
    }
    else if (command == '3') {
      battery_percent = "30%";
    }
    else if (command == '4') {
      battery_percent = "40%";
    }
    else if (command == '5') {
      battery_percent = "50%";
    }
    else if (command == '6') {
      battery_percent = "60%";
    }
    else if (command == '7') {
      battery_percent = "70%";
    }
    else if (command == '8') {
      battery_percent = "80%";
    }
    else if (command == '9') {
      battery_percent = "90%";
    }
    
  }
  wsCamera.cleanupClients();  
  sendCameraPicture(); 
  
}
