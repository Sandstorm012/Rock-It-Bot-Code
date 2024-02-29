//Haley Stewart
//Jack Charlton
//Line following with delays. In-progress
//2/29/2024

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
  pixy.line.setMode(LINE_MODE_WHITE_LINE);
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
  backLeft.writeMicroseconds(stalled);
  backRight.writeMicroseconds(stalled);
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

float timeSinceStart = millis(); //need this?
float t1L, t1R;
int Lswitch = 0, Rswitch = 0;

void loop() {
  //pixyLoop();

  timeSinceStart = millis(); //update the time since start repeatedly
  int8_t res;
  int32_t error;
  int xl = -20; //-20
  int xr = 20; //20
  
  // Get latest data from Pixy, including main vector, new intersections and new barcodes.
  res = pixy.line.getMainFeatures();
  //int Vector = pixy.line.getAllFeatures(1);
  Serial.print(res);
  //delay(100);

 //if error or nothing detected, stop motors
  if (res <= 0) {
    stop(1000); //needs to be changed to stop indefinetly?
    backward(1000);
    return;
  }

  if (res&LINE_VECTOR)
  {

    int delta_x = pixy.line.vectors->m_x1 - pixy.line.vectors->m_x0; //assign values from res for the change in x values
    //float delta_x = pixy.line.vectors[0].m_x0; //get bottom of vector
    int y0 = pixy.line.vectors->m_y0;
    int y1 = pixy.line.vectors->m_y1;
    
    // Calculate heading error with respect to m_x1, which is the far-end of the vector,
    // the part of the vector we're heading toward.
    error = (int32_t)pixy.line.vectors->m_x1 - (int32_t)X_CENTER;

    if (y1 > y0) { //the vector is going from head to tail instead of tail to head. Flip vector 
      delta_x = -delta_x;
    }

    pixy.line.vectors->print();

    // If vector is heading away and down from us (arrow pointing up), things are normal.
    if ((delta_x > xl)&&(delta_x < xr)) //implement line adjust later
    {
      forward(5); //5ms 
      //slow down a little if intersection is present so we don't miss it
    }
    else if (delta_x < xl) { //if the line is pointing to the left
      if (Lswitch == 0){
        t1L = millis(); // time 1 right
        Lswitch = 1;
      }
      
      if ((timeSinceStart - t1L > 2500)&&(Lswitch == 1)) { //take a snapshot of t1 and compare to the running clock (1 and a half seconds)
        left(5);
        Lswitch = 0;// reset Lswitch
      }
    }
    else if (delta_x > xr) { // act like a t flip flop switch
      //if (pixy.line.vectors->m_index == 1){//we saw the turn, let's take a note of the time, index = 1 is first time tracking that specific vector
        //t1R = millis(); // time 1 right
      //}
      //record t1
      if (Rswitch == 0){
        t1R = millis(); // time 1 right
        Rswitch = 1;
      }
      
      if ((timeSinceStart - t1R > 2500)&&(Rswitch == 1)) { //take a snapshot of t1 and compare to the running clock
        right(5);
        Rswitch = 0;// reset Rswitch
        // maybe add delay
      }
    }
    
  
  }
}
