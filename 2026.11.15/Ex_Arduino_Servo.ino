#define SERVO_PIN 14

#include <ESP32Servo.h>

#define SERVO_PIN 14

Servo myServo;


void setup() {
    myServo.attach(SERVO_PIN);
    myServo.write(0);
}

void loop() {
    for(int i=0;i<=180;i++)
    {
        myServo.write(i);
        delay(10);

    }
   // myServo.write(0);
    //delay(1000);
    //myServo.write(180);
    //delay(1000);
}