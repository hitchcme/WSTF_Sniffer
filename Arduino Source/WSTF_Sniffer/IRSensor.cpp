#include "Arduino.h"
#include "IRSensor.h"
  /// ******** External n counter fed through loop
  // inPin = A0

void IRSensor::setIRPin(int pwPin) {
  inPin = pwPin;
}
int IRSensor::readIRsensor(int n) {
  // Needs external n counter,
  // Read the input on analog pin 0:
  int sensorValue = analogRead(inPin);
  return sensorValue;
}
void IRSensor::printIRReadings() {
}
int IRSensor::getInPin() {
  return inPin;
}
int IRSensor::getMeanValue() {
  return mean;
}
long IRSensor::getSensorSumValue() {
  return sensorsum;
}
