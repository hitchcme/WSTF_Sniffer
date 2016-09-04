/*
#define InA1 11 // INA motor pin side 1 (right)
#define InB1 10 // INB motor pin side 1 (right)
#define PWM1 9 // PWM motor pin side 1 (right)
#define InA2 7 // INA motor pin side 2 (left)
#define InB2 6 // INB motor pin side 2 (left)
#define PWM2 5 // PM motor pin side 2 (left)
#define RELAY1 4

/*

int UD = 0;
int LR = 0;
void setup() {
Serial.begin(9600);
pinMode(InA1, OUTPUT);
pinMode(InB1, OUTPUT);
pinMode(PWM1, OUTPUT);
pinMode(InA2, OUTPUT);
pinMode(InB2, OUTPUT);
pinMode(PWM2, OUTPUT);
pinMode(RELAY1, OUTPUT);
}
void loop() {
digitalWrite(RELAY1,LOW);
UD = analogRead(A0);
LR = analogRead(A1);
if (UD > 670 && (LR > 328 && LR < 400)){ // UP
Serial.println("UP");
motorForward(200); //(25%=64; 50%=127; 100%=255)
}
else if (UD < 5 && (LR > 328 && LR < 400)){ // DOWN
Serial.println("DOWN");
motorBackward(200);
}
else if (LR < 5 && (UD > 320 && UD < 400)){ // RIGHT
Serial.println("Right");
motorRight(200);
}
else if (LR > 600 && (UD > 320 && UD < 400)){ // LEFT
Serial.println("Left");
motorLeft(200);
}
else if ((LR > 320 && LR < 400) && (UD > 320 && UD < 400)){ // IDLE
motorStop();
}
}
// switch polarity on relay
void motorLeft(int PWM_val) {
analogWrite(PWM1, PWM_val);
analogWrite(PWM2, (PWM_val)/2);
digitalWrite(InA2, LOW);

digitalWrite(InB2, HIGH);
digitalWrite(InA1, LOW);
digitalWrite(InB1, HIGH);
}
void motorRight(int PWM_val) {
analogWrite(PWM1, (PWM_val)/2);
analogWrite(PWM2, (PWM_val));
digitalWrite(InA2, LOW);
digitalWrite(InB2, HIGH);
digitalWrite(InA1, LOW);
digitalWrite(InB1, HIGH);
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

*/
