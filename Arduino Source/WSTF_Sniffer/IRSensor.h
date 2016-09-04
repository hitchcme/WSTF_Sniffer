/*
IRSensor.h Library utilizing Infrared sensor.
Created by Robert Herrera : 02/04/2016
Released into the public domain.

How to connect the sensor to the Arduino:
sensor - arduino
   vcc - 5V
   Gnd - gnd
   out - A0
    EN - Do not connect

Briefing:
This program has the sole purpose of calibrating the InfraRed sensor.
It calculates the mean of 500 values in order to converge them into
one correct value.
I am using this technique to null down the outlier values.

How to use:
After uploading this program the Arduino will start to send the mean
value of the distance through the serial port.
Don't forget to open the Serial Monitor in the Tools tab. (Ctrl +
Shift + M)
After 500 readings are made, a "RESET" string will be printed as well
as the LED will blink,
this is to allow the user to know when to move the sensor to another
distance.

*/
#ifndef IRSensor_h
#define IRSensor_h
#include "Arduino.h"

class IRSensor{
  public:
    int readIRsensor(int n);
    void setIRPin(int pwPin);
    void printIRReadings();
    int getInPin();
    int getMeanValue();
    long getSensorSumValue();
    private:
    long sensorsum = 0;
    int n = 1;
    int mean = 0;
    int lastmean = 0;
    int inPin;
};
