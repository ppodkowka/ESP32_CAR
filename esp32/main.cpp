//this is slave for esp32cam project, this program is resposible for controlling DC motors, stopping car if obsticle is in close range and getting battery %

#include <HardwareSerial.h>
#include "Arduino.h"
#include "Wire.h"
#include "Adafruit_INA219.h"

// uart
HardwareSerial SerialPort(2); 
const int rxPin = 16;
const int txPin = 17;

// char to store command which esp32-cam send
char command  = ' ';

// defining motor 1 and 2
const int motor1Pin1 = 26; 
const int motor1Pin2 = 25;  
const int motor2Pin1 = 33;  
const int motor2Pin2 = 32;

// string to store direction in which car is driving
String direction;

// defines for distance sensor
#define SOUND_SPEED 0.034
long duration;
float distanceCm;
int stopFlag = 0;
const int trigPin = 5;  
const int echoPin = 18; 

// battery tester
Adafruit_INA219 ina219;
float shuntvoltage = 0;
float busvoltage = 0;
float loadvoltage = 0;
int battery_percent = 0;
int first_measurement = 0;

// variable to store last time
unsigned long lastMillis;

// this function gets loadvoltage od battery and then turn it to battery % 
void readBatteryLevel(){
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  if(loadvoltage > 8.0){
    battery_percent = 9;
  }
  else if(loadvoltage < 8.0){
    battery_percent = 8;
  }

  SerialPort.print(battery_percent);
}

// this function stops car is distance to object is very close and car is going into that object
void checkDistance(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;

  if(distanceCm < 30 && direction == "forward"){
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    digitalWrite(motor2Pin1, LOW);
    digitalWrite(motor2Pin2, LOW);
    stopFlag = 1;
  }
}

void setup()
{
  //starting uart connection
  SerialPort.begin(9600, SERIAL_8N1, rxPin, txPin);

  // setting motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  //setting trig and echo pin as output and input
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 
}

void loop()
{
  if (SerialPort.available()){
    char command = SerialPort.read();
    if (command == '0') {
      //stop
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, LOW);
    }
    if (command == '1') {
      //forward
      direction = "forward";
      if(stopFlag == 0){
        digitalWrite(motor1Pin1, LOW); 
        digitalWrite(motor1Pin2, HIGH);
        digitalWrite(motor2Pin1, LOW); 
        digitalWrite(motor2Pin2, HIGH); 
      }
    }
    if (command == '2') {
      //backwards
      direction = "backward";
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW);
      stopFlag = 0;
    }
    if (command == '3') {
      //right
      direction = "right";
      digitalWrite(motor1Pin1, HIGH);
      digitalWrite(motor1Pin2, LOW);
      digitalWrite(motor2Pin1, LOW);
      digitalWrite(motor2Pin2, HIGH);
      stopFlag = 0;
    }
    if (command == '4') {
      //left
      direction = "left";
      digitalWrite(motor1Pin1, LOW);
      digitalWrite(motor1Pin2, HIGH);
      digitalWrite(motor2Pin1, HIGH);
      digitalWrite(motor2Pin2, LOW);
      stopFlag = 0;
    }
  }

  checkDistance();

  // this happens in the beggining just to take starting battery percentage moze nawet 1 sekunde dac - do sprawdzenia 
  if(millis() < 2*1000UL){
    readBatteryLevel();
  }

  // this part updates battery percentage every 3 minutes
  if (millis() - lastMillis >= 3*60*1000UL) {
   lastMillis = millis();  //get ready for the next iteration
   readBatteryLevel();
  }
}
