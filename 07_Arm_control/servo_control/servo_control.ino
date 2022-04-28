
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  150
#define SERVOMAX  600
#define USMIN  500
#define USMAX  2400
#define SERVO_FREQ 50

void setup() {
  pwm.begin();

  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);

  delay(1000);

  pwm.writeMicroseconds(4, map(95, 0, 180, 500, 2400));  // contorl S5 servo
  pwm.writeMicroseconds(5, map(96, 0, 180, 500, 2400));  // contorl S6 servo
  pwm.writeMicroseconds(6, map(10, 0, 180, 500, 2400));  // contorl S7 servo
  pwm.writeMicroseconds(7, map(35, 0, 180, 500, 2400));  // contorl S8 servo

}

void loop() {

}
