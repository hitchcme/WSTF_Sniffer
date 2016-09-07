#include "Arduino.h"
#include "UltraSonicSensor.h"

void UltraSonicSensor::setUltraSonicSensorPin(int pwPin) { //setup
//Digital pin 7 for reading in the pulse width from the MaxSonar device.
//This variable is a constant because the pin will not change throughout execution of this code.
//const int pwPin = 4;
//variables needed to store values
  inPin = pwPin;
  pinMode(inPin,INPUT);
}
void UltraSonicSensor::setPulseReader(int pwPin) { //loop
  //Used to read in the pulse that is being sent by the MaxSonar device.
  //Pulse Width representation with a scale factor of 147 uS per Inch.
  pulse = pulseIn(pwPin, HIGH);
  //147uS per inch
  inches = pulse/147;
  //change inches to centimetres
  cm = inches * 2.54;
}
void UltraSonicSensor::printSonicReadings() {
  Serial.print("Inches: ");
  Serial.print(inches);
  Serial.print("cm: ");
  Serial.print(cm);
}
int UltraSonicSensor::getInPin() {
  return inPin;
}
long UltraSonicSensor::getPulseValue() {
  return pulse;
}
long UltraSonicSensor::getInchesValue() {
  return inches;
}
long UltraSonicSensor::getCMValue() {
  return cm;
}
