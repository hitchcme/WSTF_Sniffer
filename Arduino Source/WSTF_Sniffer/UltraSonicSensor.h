/*
UltraSonicSensor.h Library
utilizing Ultra sonic sensor.
Created by Robert Herrera : 02/04/2016
Released into the public domain.
*/
#ifndef UltraSonicSensor_h
#define UltraSonicSensor_h
#include "Arduino.h"
class UltraSonicSensor{
public:
void setUltraSonicSensorPin(int pwPin);
void setPulseReader(int pwPin);
void printSonicReadings();
int getInPin();
long getPulseValue();
long getInchesValue();
long getCMValue();
private:
int inPin; // pin number
long pulse; // PWD value
long inches; // inches value converted from PWD value
long cm; // cm value converted from inches value
};
#endif