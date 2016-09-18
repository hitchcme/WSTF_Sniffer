// Anything...

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

//String sentence = String(100);

//char sentence[sentenceSize];

// Function intros
void receiveEvent(int howMany);
void motorLeft(int PWM_val);
void motorRight(int PWM_val);
void motorForward(int PWM_val);
void motorBackward(int PWM_val);
void motorStop();
void emergencySoftwareStop();
void displayGPS();
void getField(char* buffer, int index);
void readGPS(int i);


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
//	delay(25);
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
//	Serial.print("MoL");
	analogWrite(PWM1, (PWM_val));
	analogWrite(PWM2, (PWM_val));
	digitalWrite(InA2, HIGH);
	digitalWrite(InB2, LOW);
	digitalWrite(InA1, LOW);
	digitalWrite(InB1, HIGH);

}

void motorRight(int PWM_val) {
//	Serial.print(",MoR,");
	analogWrite(PWM1, (PWM_val));
	analogWrite(PWM2, (PWM_val));
	digitalWrite(InA2, LOW);
	digitalWrite(InB2, HIGH);
	digitalWrite(InA1, HIGH);
  	digitalWrite(InB1, LOW);

}


void motorForward(int PWM_val) {
//	Serial.print(",MoF,");
	analogWrite(PWM1, PWM_val);
	analogWrite(PWM2, PWM_val);
	digitalWrite(InA2, LOW);
	digitalWrite(InB2, HIGH);
	digitalWrite(InA1, LOW);
	digitalWrite(InB1, HIGH);

}


void motorBackward(int PWM_val) {
//	Serial.print(",MoB,");
	analogWrite(PWM1, PWM_val);
	analogWrite(PWM2, PWM_val);
	digitalWrite(InA1, HIGH);
	digitalWrite(InB1, LOW);
	digitalWrite(InA2, HIGH);
	digitalWrite(InB2, LOW);

}


void motorStop() {
//	Serial.print(",MoS,");
	analogWrite(PWM1, 0);
	analogWrite(PWM2, 0);
	digitalWrite(InA1, LOW);
	digitalWrite(InB1, LOW);
	digitalWrite(InA2, LOW);
	digitalWrite(InB2, LOW);

}


void emergencySoftwareStop(){
//	Serial.print(",MoES,");
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
	if (strcmp(field, "$GPGGA") == 0) {

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
		Serial.println("\n");

	}

}// end displayGPS



// parses serial info by field
void getField(char* buffer, int index){
	String sentence;
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

void readGPS(int i) {
	//initialize the variables
	// the sentence declaration might need further definition, for size
	// 	and maybe it needs to be reymoved from the global declarations,
	//	if it exists globally.
	String sentence;
	int sentenceNum = 0;
	int deadAir = 0;
	char ch = '\0';
	if (gpsSerial.available()) {
		ch = gpsSerial.read();
		if (ch == 36){   //look for $ and begin loop
			Serial.print("\n");
			sentenceNum++;
			//build sentence, if $ appears start a new sentence
			for (int k = 0 ; k < 162; k++ ) {
				ch = gpsSerial.read();
				if (ch >= 36 && ch < 129){
					if (ch == 36){
						Serial.println();
						k=0;
						sentenceNum++;
					}
					sentence +=ch;
					Serial.print(ch);
				}
			// Trying to catch the time gap to know when to
			// get a sentence reference
			//	if (ch == '\0'){
			//		deadAir++;
			//	}
			//	if (deadAir > 10){
			//		sentenceNum = 0;
			//		Serial.print("  DEAD AIR");
			//	}
				else k--;
			}
		}
		// Looking for the money!!!
		// While no money, keep looping
//		while ( ch > 31 && ch < 129 && ch !=36 ) {
//			ch = gpsSerial.read();
//			Serial.print(ch);
//		}
		// BAAMMMM!, now we have money!!!
//		sentence += ch; // Append money to empty string
//		ch = gpsSerial.read(); //reset ch, so it doesn't screw up the next bits
		// While not newline, keep appending the characters
		//	hopefully we dont miss any characters!
//		while( ch > 31 && ch < 129 && ch !=36 ) {
//			ch = gpsSerial.read();
//			if ( ch > 31 && ch < 129 && ch != 36 ) {
				// keeping this inside here, just in case we find money, where we dont want it!
//				sentence += ch;
//			}
//		}
		// And this should be a complete NMEA sentence
//		Serial.print(sentence);
//		Serial.println();
		// Add a \n to the sentence, if it doesn't get appended to the sentence
	}
} //end readGPS
