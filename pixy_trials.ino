//Haley Stewart
//2/19/2024
// Trials to find right camera width for Pixy2

#include <Servo.h>
#include <Pixy2.h>
//#include Line_tracking.ino

//Pixy2 pixy;

Pixy2 pixy;

void pixySetup()
{
  Serial.begin(115200);
  Serial.print("Starting...\n");

  // initialize the pixy object
  pixy.init();
  // Full brightness
  pixy.setLamp(1, 1);
  // Change to line tracking program
  pixy.changeProg("line");
  // instruct the pixy to follow lighter lines on darker backgrounds and not vice versa
  //int8_t setMode(uint8_t LINE_MODE_WHITE_LINE)
}

void pixyLoop()
{
  int8_t i;
  char buf[128];
 
  pixy.line.getAllFeatures();

  // print all vectors
  for (i=0; i<pixy.line.numVectors; i++)
  {
    sprintf(buf, "line %d: ", i);
    Serial.print(buf);
    pixy.line.vectors[i].print();
  }
  
  // print all intersections
  for (i=0; i<pixy.line.numIntersections; i++)
  {
    sprintf(buf, "intersection %d: ", i);
    Serial.print(buf);
    pixy.line.intersections[i].print();
  }

  // print all barcodes
  for (i=0; i<pixy.line.numBarcodes; i++)
  {
    sprintf(buf, "barcode %d: ", i);
    Serial.print(buf);
    pixy.line.barcodes[i].print();
  }
}

#define X_CENTER         (pixy.frameWidth/2)

Servo frontLeft; //drive, servo is running slower than the others at the same speed
Servo frontRight; //drive
Servo backLeft;
Servo backRight;
Servo tilt;

int servoPinFL = 4; //left -> needs to go ccw to go "forward"
int servoPinFR = 3; //right -> needs to go cw to go "forward"
int servoPinBL = 5; 
int servoPinBR = 6;
int servoPinTilt = 2;

int pos1 = 1500; //store frontLeft stopped position
int pos2 = 1500; //store frontRight stopped position
int pos3 = 1500;
int pos4 = 1500;


int clockwise = 600; //normally 500
int stalled = 1500; 
int counterClockwise = 2400; //normally 2500

void forward(int runtime) {
  frontLeft.writeMicroseconds(2500); //left ccw
  frontRight.writeMicroseconds(clockwise); //right clockwise is 500
  backLeft.writeMicroseconds(counterClockwise); //cclockwise is 2500
  backRight.writeMicroseconds(clockwise);
  delay(runtime);
}

void backward(int runtime) {
  frontLeft.writeMicroseconds(clockwise); //left cw
  frontRight.writeMicroseconds(counterClockwise); //right ccw 
  backLeft.writeMicroseconds(clockwise);
  backRight.writeMicroseconds(counterClockwise);
  delay(runtime);
}

void left(int runtime) { //correct, moves fast
  frontLeft.writeMicroseconds(clockwise); 
  frontRight.writeMicroseconds(clockwise); 
  backLeft.writeMicroseconds(clockwise); 
  backRight.writeMicroseconds(clockwise);
  delay(runtime);
}

void right(int runtime) {
  frontLeft.writeMicroseconds(counterClockwise); 
  frontRight.writeMicroseconds(counterClockwise); 
  backLeft.writeMicroseconds(counterClockwise);
  backRight.writeMicroseconds(counterClockwise);
  delay(runtime);
}

void stop(int runtime) {
  frontLeft.writeMicroseconds(stalled);
  frontRight.writeMicroseconds(stalled);
  backLeft.writeMicroseconds(stalled); //1500
  backRight.writeMicroseconds(stalled);
  delay(runtime);
}

void slow(int runtime) {
  frontLeft.writeMicroseconds(2400); //left ccw
  frontRight.writeMicroseconds(700); //right clockwise is 600
  backLeft.writeMicroseconds(2300); //cclockwise is 2400
  backRight.writeMicroseconds(700);
  delay(runtime);
}

//distances are 3 average values over the span of 30ms from ultrasonic sensors. fake input data below in inches:
int leftDistance = 5; //left ultrasonic sensor
int middleDistance = 10; //middle ultrasonic sensor
int rightDistance = 4; //right ultrasonic sensor

void course() {
  forward(10);
  delay(5900); //need to debug delays //17s 1000ms in 1 sec //9.13
  left(10);
  delay(1500); //2
  forward(10);
  delay(2000);
  right(10);
  delay(1500);
  left(10);
  delay(1500);
  delay(1500);
  forward(10);
  delay(2000);                                                                                                                                        
  stop(10);
}

void servoInit() {
  //Initialize servos
  frontLeft.attach(servoPinFL);
  frontRight.attach(servoPinFR);
  backLeft.attach(servoPinBL);
  backRight.attach(servoPinBR);
  tilt.attach(servoPinTilt);
  tilt.write(70); //going down in number tilts up 70
}

void setup() {
  servoInit();
  pixySetup(); //hope this works
  //pixyLoop();

  //test only once
  //test();

  //forward(1500);
  //right(10);
  //delay(950);
  //left();
  //delay(2200);
  //delay(1500);
  //forward();
  //delay(2000);
  //stop(10);

}



void loop() {
  
  //pixyLoop();

  int8_t res;
  int32_t error;
  int xl = -5; //-20
  int xr = 5; //20
  // Get latest data from Pixy, including main vector, new intersections and new barcodes.
  res = pixy.line.getMainFeatures();
  
 //if error or nothing detected, stop motors
  if (res <= 0) {
    stop(10); //needs to be changed to stop indefinetly?
    return;
  }

  //we found the vector
  if (res&LINE_VECTOR)
  {
    int delta_x = pixy.line.vectors->m_x1 - pixy.line.vectors->m_x0; //assign values from res for the change in x values
    int y0 = pixy.line.vectors->m_y0;
    int y1 = pixy.line.vectors->m_y1;

    if (y1 > y0) { //the vector is going from head to tail instead of tail to head. Flip vector 
      delta_x = -delta_x;
    }
    // Calculate heading error with respect to m_x1, which is the far-end of the vector,
    // the part of the vector we're heading toward.
    error = (int32_t)pixy.line.vectors->m_x1 - (int32_t)X_CENTER;

    pixy.line.vectors->print();

    while ((xl > -39)&&(xr < 39)) { //if we are inside of the range, do test
      int counter = 0;
      // If vector is heading away and down from us (arrow pointing up), things are normal.
      while (counter < 2000) { //while timer less than 10 sec 10000ms 
        if ((delta_x > xl)&&(delta_x < xr)) //implement line adjust later
        {
          forward(5); //5ms 
          //slow down a little if intersection is present so we don't miss it
        }
        else if (delta_x < xl) { //if the line is pointing to the left
          left(5);
        }
        else (delta_x > xr) {
          right(5); //
        }
        counter += 5; // 5 ms passed
      }

      //increase counter and wait 5s before next run
      //delay(10000);
      stop(5000); //5s intermission
      xl -= 5;
      xr += 5;
      //delay(5000);

    }

  }
  // if there is an intersection, turn left

}
