/*
int origin[] = {95, 96, 10, 35};
int down[] = {95, 96, 40, 35};
int cut[] = {95, 96, 40, 100};
int up[] = {95, 96, 10, 100};
int turn1[] = {5, 96, 10, 100};
int turn2[] = {48, 96, 10, 100};
int down1[] = {5, 96, 40, 100};
int down2[] = {48, 96, 40, 100};
int drop1[] = {5, 96, 40, 35};
int drop2[] = {48, 96, 40, 35};
int return1[] = {5, 96, 10, 35};
int return2[] = {48, 96, 10, 35};
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150
#define SERVOMAX  600
#define USMIN  500
#define USMAX  2400
#define SERVO_FREQ 50

int vr = 0;
int bt = 0;
int choose = 4;
int mov = 95;
int don = 5;

String inputString = "";         // a String to hold incoming data

void setup() {
  Serial.begin(9600);
  pinMode(5, INPUT_PULLUP);
  
  pwm.begin();

  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);

  delay(10);

  servoList(95, 96, 5, 35);
  delay(1000);
  
}

void loop() {
  if (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      if (inputString == "A\r") {
        class_box(1);
      }
      else {
        class_box(2);
      }
      inputString = "";
    }
    else {
      inputString += inChar;
    }
  }

  if (digitalRead(5) == 0) {
    while (digitalRead(5) == 0) {}
    Serial.println("I");
  }
}


void servo(byte pins, byte deg) {
  if (deg < 0) deg = 0;
  if (deg > 180) deg = 180;
  pwm.writeMicroseconds(pins, map(deg, 0, 180, 500, 2400));
}

void servoList(byte deg1, byte deg2, byte deg3, byte deg4) {
  pwm.writeMicroseconds(5, map(deg2, 0, 180, 500, 2400));
  
  pwm.writeMicroseconds(7, map(deg4, 0, 180, 500, 2400));
  if (don == deg3) {
    
  }
  else if (don < deg3){
    for (int i=don; i<=deg3; i++) {
      pwm.writeMicroseconds(6, map(i, 0, 180, 500, 2400));
      delay(10);
    }
  }
  else {
    for (int i=don; i>=deg3; i--) {
      pwm.writeMicroseconds(6, map(i, 0, 180, 500, 2400));
      delay(10);
    }
  }
  don = deg3;
  
  if (mov == deg1) {
    
  }
  else if (mov < deg1){
    for (int i=mov; i<=deg1; i++) {
      pwm.writeMicroseconds(4, map(i, 0, 180, 500, 2400));
      delay(10);
    }
  }
  else {
    for (int i=mov; i>=deg1; i--) {
      pwm.writeMicroseconds(4, map(i, 0, 180, 500, 2400));
      delay(10);
    }
  }
  mov = deg1; 
  delay(1000);
}

void class_box(int action) {
  if (action == 1) {
    servoList(95, 96, 40, 35);
    servoList(95, 96, 40, 100);
    servoList(95, 96, 5, 100);
    servoList(48, 96, 5, 100);
    servoList(48, 96, 40, 100);
    servoList(48, 96, 40, 35);
    servoList(48, 96, 5, 35);
    servoList(95, 96, 5, 35);
  }
  else {
    servoList(95, 96, 40, 35);
    servoList(95, 96, 40, 100);
    servoList(95, 96, 5, 100);
    servoList(5, 96, 5, 100);
    servoList(5, 96, 40, 100);
    servoList(5, 96, 40, 35);
    servoList(5, 96, 5, 35);
    servoList(95, 96, 5, 35);
  }
}
