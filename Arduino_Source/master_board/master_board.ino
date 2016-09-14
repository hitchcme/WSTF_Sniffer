//Arduino Master
//i2c Master Code(UNO)
// Master Arduino 1 ( IR Sensors, Sonar Sensors)
#include "IRSensor.h"
#include "UltraSonicSensor.h"
/*
* Master arduinio 1 is soley in charge of reading sensor data.
* If it is determined that sensors data is picking up and appraoching
* obstacle, then a motion stop will be triggered via serial communication to
* the GPS/MotorDriver Arduino.
*/
#include <Wire.h>
char c;
/* IR Sensors */
// initialize Infrared Sensors
IRSensor frontRightSensor = IRSensor();
IRSensor frontLeftSensor = IRSensor();
IRSensor rightSensor = IRSensor();
IRSensor leftSensor = IRSensor();
IRSensor backRightSensor = IRSensor();
IRSensor backLeftSensor = IRSensor();
/* UltraSonic Sensors */
// initialize Infrared Sensors
UltraSonicSensor frontSensor = UltraSonicSensor(); // needs numbers
UltraSonicSensor backSensor = UltraSonicSensor();
int flag = 0;
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * *
* IR Sensor Counters (used to
calculate mean distance) (Per sensor)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * */
int n = 1; // counter
int decision = 0; // decision variable that determines direction after peripheral reading
int frontRightSensorValue;
long sensorsum = 0;
int mean = 0;
int lastmean = 0;
int frontLeftSensorValue;
long sensorsum2 = 0;
int mean2 = 0;
int lastmean2 = 0;
int rightSensorValue;
long sensorsum3 = 0;
int mean3 = 0;
int lastmean3 = 0;
int leftSensorValue;
long sensorsum4 = 0;
int mean4 = 0;
int lastmean4 = 0;
int backRightSensorValue;
long sensorsum5 = 0;
int mean5 = 0;
int lastmean5 = 0;
int backLeftSensorValue;
long sensorsum6 = 0;
int mean6 = 0;
int lastmean6 = 0;
//
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * *
* Initialization variables
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * * * * * * * */
int referenceAngle;
int start;

void resetSensorSums();
void executeDecision(int n);
int readFrontPeripherals();    // Corrected spelling by Rusty 9/5/16
void readIRSensors();
void incrementSensorSums();



void setup(){
	Serial.begin(9600);
	Wire.begin();
	start = 0; // when first uploaded, queues initialization phase before reading
	/*
	// set pin numbers to Sonic sensors
	frontSensor.setUltraSonicSensorPin(9); // sets pin number to pin 9
	backSensor.setUltraSonicSensorPin(8); // sets pin number to pin 8
	// 9 front cal vals : 13,14
	// 8 back cal vals : 11,12
	//
	* A0 = Front Right
	* A1 = Front Left
	* A2 = Back Right
	* A3 = Back Left
	* A4 = Left
	* A5 = Right
	*  2 = US Back
	*  3 = US Front
	*/
	//set desired Pin numbers to IR Pins
	frontRightSensor.setIRPin(A0);
	frontLeftSensor.setIRPin(A1);
	backRightSensor.setIRPin(A2);
	backLeftSensor.setIRPin(A3);
	leftSensor.setIRPin(A4);
	rightSensor.setIRPin(A5);
	frontSensor.setUltraSonicSensorPin(2);
	backSensor.setUltraSonicSensorPin(3);
}    // end setup()

void loop() {
	// The supposed index????
	//int i; // index
	// read front and back sensor
	frontSensor.setPulseReader(9); // initializes front sensor pulsewidth reading
	backSensor.setPulseReader(8); // initializes back sensor pulsewidth reading
	bool isObstacle;
	int frontVal = frontSensor.getInchesValue();

	if(frontVal == 12 || frontVal == 13 || frontVal == 14) {
		isObstacle = false;
	}
	else {
		isObstacle = true;
	}
	delay(200);
	readIRSensors();
	incrementSensorSums();
	// While condition that checks if there is a serial connection or an
        // obstacle as long as at least one condition is met (G S B R L)
	while(Serial.available() || isObstacle == true) {
		if( c!= 71 || c!= 83 || c!= 66 || c!= 82 || c!= 76)//G|S|B|R|L
			c = Serial.read();
			c = 'G';
		   // Master Transimission to arduino 2 (5)
		if(isObstacle == true) {   //stop
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			decision = readFrontPeripherals();
			executeDecision(decision);
			isObstacle = false;
		}
		if(c == 'G') {
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
		}
		if (c == 'S' && isObstacle == false) {
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			isObstacle = false;
		}
		if(c == 'B'){    // backwards
			Wire.beginTransmission(5);
			Wire.write('B');
			Wire.endTransmission();
		}
		if(c == 'R'){    //Right
			Wire.beginTransmission(5);
			Wire.write('R');
			Wire.endTransmission();
		}
		if(c == 'L'){    // Left
			Wire.beginTransmission(5);
			Wire.write('L');
			Wire.endTransmission();
			}
			// Master Transimission to arduino 3 (9)
		if(c == 'h'){
			Wire.beginTransmission(9);
			Wire.write('H');
			Wire.endTransmission();
		}
		else if(c == 'l'){
			Wire.beginTransmission(9);
			Wire.write('L');
			Wire.endTransmission();
		}
		if(isObstacle == false) {
		}
	// else transmit stop
	}    // end while loop
	// is this incremental counter supposed to be inside the last if statement?
	n = n + 1;
	if (n > 500) {
		resetSensorSums();
	}
	// The "Mean" value will vary the whole loop because it is always calculating the mean with the read values
	// The "Last Mean" value will only show the calculated mean value just to ease the reading of the calculated value

//	Serial.print("\nFLMC,");
//	Serial.print(mean,DEC);
//	Serial.print(",FLML,");
//	Serial.print(lastmean,DEC);
//	Serial.print(",FRMC,");
//	Serial.print(mean2,DEC);
//	Serial.print(",FR_ML,");
//	Serial.print(lastmean2);
//	Serial.print(",BL_MC,");
//	Serial.print(mean5,DEC);
//	Serial.print(",BL_ML,");
//	Serial.print(lastmean5,DEC);
//	Serial.print(",BR_MC,");
//	Serial.print(mean6,DEC);
//	Serial.print(",BR_ML,");
//	Serial.print(lastmean6,DEC);
//	Serial.print(",IR_FR,");
	Serial.print("\n*******************************************************************************************\n\n");
	Serial.print("OUTPUTTING SENSOR VALUES\n");
	Serial.print("IR_FR,");
	Serial.print(frontRightSensorValue,DEC);
	Serial.print(",IR_FL,");
	Serial.print(frontLeftSensorValue,DEC);
	Serial.print(",IR_BR,");
	Serial.print(backRightSensorValue,DEC);
	Serial.print(",IR_BL,");
	Serial.print(backLeftSensorValue,DEC);
	Serial.print(",IR_LL,");
	Serial.print(leftSensorValue,DEC);
	Serial.print(",IR_RR,");
	Serial.print(rightSensorValue,DEC);
	Serial.print(",U_FF,");
	frontSensor.printSonicReadings();
	Serial.print(",U_BB,");
	backSensor.printSonicReadings();
	Serial.print(",STAT,");
	Serial.println(c);
	delay(25);
	Serial.print("\n\n******************************************************************************************* \n");
	if(start > 500) {
//		Serial.println("RESET");
		start = 1;
	}
	start++;
}// end void loop()

void executeDecision(int n) {
	switch(n) {
		case 1:
			Serial.print("\n******************************************************************************************** \n\n");
			Serial.print("INITIATE DECISION 1\n");
			Serial.print("R(900) >> S(900) >> G(900) >> L(900) >> G(2000) >> S(2000).");
			Wire.beginTransmission(5);
			Wire.write('R');
			Wire.endTransmission();
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('L');
			Wire.endTransmission();
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			delay(2000);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			delay(2000);
			Serial.print("\n\n******************************************************************************************* \n");
			break;
		case 2:
			Serial.print("\n******************************************************************************************* \n\n");
			Serial.print("INITIATE DECISION 2\n");
			Wire.beginTransmission(5);
			Wire.write('L');
			Wire.endTransmission();
			Serial.print("L(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('R');
			Wire.endTransmission();
			Serial.print(">> R(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(2000) ");
			delay(2000);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(2000) ");
			delay(2000);
			Serial.print("\n\n******************************************************************************************* \n");
			break;
		case 3:
			Serial.print("\n******************************************************************************************* \n\n");
			Serial.print("INITIATE DECISION 3\n");
			Wire.beginTransmission(5);
			Wire.write('B');
			Wire.endTransmission();
			Serial.print("B(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(3000) ");
			delay(3000);
			Serial.print("\n\n******************************************************************************************* \n");
			break;
		case 4:
			Serial.print("\n******************************************************************************************* \n\n");
			Serial.print("INITIATE DECISION 4\n");
			Wire.beginTransmission(5);
			Wire.write('R');
			Wire.endTransmission();
			Serial.print("R(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('L');
			Wire.endTransmission();
			Serial.print(">> L(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(900) ");
			delay(2000);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(2000) ");
			delay(2000);
			Serial.print("\n\n******************************************************************************************* \n");
			break;
		default:
			Serial.print("\n******************************************************************************************* \n\n");
			Serial.print("INITIATE DEFAULT DECISION\n");
			Wire.beginTransmission(5);
			Wire.write('R');
			Wire.endTransmission();
			Serial.print("R(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('L');
			Wire.endTransmission();
			Serial.print(">> L(900) ");
			delay(900);
			Wire.beginTransmission(5);
			Wire.write('G');
			Wire.endTransmission();
			Serial.print(">> G(2000) ");
			delay(2000);
			Wire.beginTransmission(5);
			Wire.write('S');
			Wire.endTransmission();
			Serial.print(">> S(2000) ");
			delay(2000);
			Serial.print("\n\n******************************************************************************************* \n");
			break;
	}    // end switch
}    // end executeDecision

int readFrontPeripherals() {
	// if frontLeft occupied
	if ( mean > lastmean && mean2 == lastmean2) {
		return 1;
	}
	// if frontRight occupied
	else if ( mean2 > lastmean2 && mean == lastmean) {
		return 2;
	}
	// if both sensors occupied
	else if ( mean2 > lastmean2 && mean > lastmean) {
		return 3;
	}
	// if both open
	else if ( mean2 == lastmean2 && mean == lastmean) {
		return 1;
	}
	return 0;
}

void readIRSensors() {
	frontRightSensorValue = analogRead(A0);
	frontLeftSensorValue = analogRead(A1);
	backRightSensorValue = analogRead(A2);
	backLeftSensorValue = analogRead(A3);
	// Rusty adding back in left and right sensors 9/5/16
	leftSensorValue = analogRead(A4);
	rightSensorValue = analogRead(A5);
}

void incrementSensorSums() {
	lastmean = mean;
	// Calculate mean of measures of a specified distance in order to smooth
	// the results and after create the regression line for distance
	sensorsum = (sensorsum + frontLeftSensorValue);
	mean = (sensorsum / n);
	lastmean2 = mean2;
	// Calculate mean of measures of a specified distance in order to smooth
	// the results and after create the regression line for distance
	sensorsum2 = (sensorsum2 + frontRightSensorValue);
	mean2 = (sensorsum2 / n);
	lastmean5 = mean5;
	// Calculate mean of measures of a specified distance in order to smooth
	// the results and after create the regression line for distance
	sensorsum5 = (sensorsum5 + backRightSensorValue);
	mean5 = (sensorsum5 / n);
	lastmean6 = mean6;
	// Calculate mean of measures of a specified distance in order to smooth
	// the results and after create the regression line for distance
	sensorsum6 = (sensorsum6 + backRightSensorValue);
	mean6 = (sensorsum6 / n);
}
void resetSensorSums() {
	delay(250);
	n = 1;
	sensorsum = 0;
	lastmean = mean;
	n = 1;
	sensorsum2 = 0;
	lastmean2 = mean2;
	n = 1;
	sensorsum5 = 0;
	lastmean5 = mean5;
	n = 1;
	sensorsum6 = 0;
	lastmean6 = mean6;
}