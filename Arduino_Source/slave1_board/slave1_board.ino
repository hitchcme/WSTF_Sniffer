#include <Wire.h> // serial i2c communication
//datalogger library
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Motor Driver Pin Definitions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#define InA1 11 // INA motor pin side 1 (right)
#define InB1 10 // INB motor pin side 1 (right)
#define PWM1 9 // PWM motor pin side 1 (right)
#define InA2 7 // INA motor pin side 2 (left)
#define InB2 6 // INB motor pin side 2 (left)
#define PWM2 5 // PWM motor pin side 2 (left)
#define RELAY1 4 // Relay connection


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* GPS
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
SoftwareSerial gpsSerial(2, 3); // RX, TX (TX not used) digital pins 2,3
const int sentenceSize = 80;
// the $GPGGA, $GPGSA, etc. are sentences and are sent 1 character at a time from the GPS
char sentence[sentenceSize];
void setup() {
pinMode(InA1, OUTPUT);
pinMode(InB1, OUTPUT);
pinMode(PWM1, OUTPUT);
pinMode(InA2, OUTPUT);
pinMode(InB2, OUTPUT);
pinMode(PWM2, OUTPUT);
pinMode(RELAY1, OUTPUT);
digitalWrite(RELAY1,HIGH);
Serial.begin(9600);
gpsSerial.begin(9600); // begin GPS Serial Communication at default baud rate
Wire.begin(5); // begin serial communication :: Define current Arduino address as 5
Wire.onReceive(receiveEvent); // serial subroutine
}
void loop() {
int i;
readGPS(i);
delay(500);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Serial Communication
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// serial subroutine ( accepts motor commands relayed from master arudino )
void receiveEvent(int howMany) {
  while(Wire.available()){
    char c = Wire.read();
    if(c == 'G'){
      motorForward(80);
    }
    else if(c == 'S'){
      motorStop();
    }
    else if(c == 'B'){
      motorBackward(80);
    }
    else if(c == 'R'){
      motorRight(61);
    }
    else if(c == 'L'){
      motorLeft(61);
    }
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Motor Driver Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// switch polarity on relay
  void motorLeft(int PWM_val) {
  analogWrite(PWM1, (PWM_val));
  analogWrite(PWM2, (PWM_val));
  digitalWrite(InA2, HIGH);
  digitalWrite(InB2, LOW);
  digitalWrite(InA1, LOW);
  digitalWrite(InB1, HIGH);
}
void motorRight(int PWM_val) {  
  analogWrite(PWM1, (PWM_val));
  analogWrite(PWM2, (PWM_val));
  digitalWrite(InA2, LOW);
  digitalWrite(InB2, HIGH);
  digitalWrite(InA1, HIGH);
  digitalWrite(InB1, LOW);
}
    void motorForward(int PWM_val) {
    analogWrite(PWM1, PWM_val);
    analogWrite(PWM2, PWM_val);
    digitalWrite(InA2, LOW);
    digitalWrite(InB2, HIGH);
    digitalWrite(InA1, LOW);
    digitalWrite(InB1, HIGH);
}
void motorBackward(int PWM_val) {
  analogWrite(PWM1, PWM_val);
  analogWrite(PWM2, PWM_val);
  digitalWrite(InA1, HIGH);
  digitalWrite(InB1, LOW);
  digitalWrite(InA2, HIGH);
  digitalWrite(InB2, LOW);
}
void motorStop() {
  analogWrite(PWM1, 0);
  analogWrite(PWM2, 0);
  digitalWrite(InA1, LOW);
  digitalWrite(InB1, LOW);
  digitalWrite(InA2, LOW);
  digitalWrite(InB2, LOW);
}
void emergencySoftwareStop(){
  while (true){
    digitalWrite(RELAY1,HIGH);
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* GPS Functions
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void displayGPS(){
  char field[20];
  getField(field, 0);
  
  // current field by lat.,long. etc.
  if (strcmp(field, "$GPGGA") == 0){
    Serial.print("Lat: ");
    getField(field, 2); // number
    Serial.print(field);
    getField(field, 3); // N/S
    Serial.print(field);
    Serial.print(" Long: ");
    getField(field, 4); // number
    Serial.print(field);
    getField(field, 5); // E/W
    Serial.println(field);
    Serial.print("Altitude: ");
    getField(field, 9);
    Serial.println(field);
    Serial.print("Number of satellites: ");
    getField(field, 7);
    Serial.println(field);
    Serial.println("\
    n");
  }
}// end displayGPS

// parses serial info by field
void getField(char* buffer, int index){
  int sentencePos = 0;
  int fieldPos = 0;
  int commaCount = 0;
  while (sentencePos < sentenceSize){
    if (sentence[sentencePos] == ','){
      commaCount ++;
      sentencePos ++;
    }
    if (commaCount == index){
      buffer[fieldPos] = sentence[sentencePos];
      fieldPos ++;
    }
    sentencePos ++;
  }
  buffer[fieldPos] = '\0';
}// end getField

void readGPS(int i){
  if (gpsSerial.available()){
     Serial.print("GPS Serial is available\n");
    char ch = gpsSerial.read();
    if (ch != '\n' && i < sentenceSize){
      sentence[i] = ch;
      i++;
    }
    else{
      sentence[i] = '\0';
      i = 0;
      Serial.println("\n");
      Serial.println(sentence);
      Serial.println("\n");
      displayGPS();
    }
  }
} //end readGPS
