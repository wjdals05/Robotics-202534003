#define SERVO_PIN 14
#define POT_PIN 39

#include <ESP32Servo.h>



Servo myServo;


void setup() {
    myServo.attach(SERVO_PIN);
    myServo.write(0);
}

void loop() {
    int value= analogRead(POT_PIN);
    myServo.write(value);
    delay(10);
 }

