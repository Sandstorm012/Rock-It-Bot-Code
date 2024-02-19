//Haley Stewart
//Basic motor control with forward, backward, left and right movement of two motors.
//Inputs sample distance data from ultrasonic sensors and adjusts accordingly

#include <Servo.h>

Servo servo1;
Servo servo2;

int servoPin1 = 3; //left -> needs to go ccw to go "forward"
int servoPin2 = 2; //right -> needs to go cw to go "forward"

int pos1 = 1500; //store servo1 stopped position
int pos2 = 1500; //store servo2 stopped position

int clockwise = 500;
int stalled = 1500; 
int counterClockwise = 2500;

void forward() {
  servo1.writeMicroseconds(counterClockwise); //left ccw
  servo2.writeMicroseconds(clockwise); //right cw
}

void backward() {
  servo1.writeMicroseconds(clockwise); //left cw
  servo2.writeMicroseconds(counterClockwise); //right ccw 
}

void left() {
  servo1.writeMicroseconds(stalled); //left wheel stop
  servo2.writeMicroseconds(clockwise); //right wheel forward
}

void right() {
  servo1.writeMicroseconds(counterClockwise); //left wheel forward
  servo2.writeMicroseconds(stalled); //right wheel stop
}

void stop() {
  servo1.writeMicroseconds(stalled);
  servo2.writeMicroseconds(stalled);
}

void test() {
  forward();
  delay(1500);
  backward();
  delay(1500);
  left();
  delay(1500);
  right();
  delay(1500);
  stop();
  delay(2000);
}

//distances are 3 average values over the span of 30ms from ultrasonic sensors. fake input data below in inches:
int leftDistance = 5; //left ultrasonic sensor
int middleDistance = 10; //middle ultrasonic sensor
int rightDistance = 4; //right ultrasonic sensor

void setup() {
  //Initialize servos
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  //test only once
  //test();
}

void loop() {
  //start prototype loop
  //1 revolution a second?
  if (middleDistance > 4) { //while there is a clear path forward, 
    if ((leftDistance > 4) && (rightDistance > 4)) { //nothing too close to left or right
      forward(); //continue forward
      delay(1500);
    }
    if (leftDistance <= 4) { //wall or object is detected to left
      right(); //adjust right
      delay(1500); //duration to adjust right away from obstacle 
      leftDistance += 1;
    }
    if (rightDistance <= 4) { //wall or object is detected to right
      left(); //adjust left
      delay(1500);
      rightDistance += 1;
    }
    middleDistance -= 1; //distance gradually going down as robot progresses forward
  }
  else { //if something in front, stop and reverse and turn to left, for example
    stop();
    delay(1500);
    backward();
    delay(1500);
    left();
    delay(1500);
    middleDistance = 10;
  }
  

}
