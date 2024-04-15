//GoBabyGo Wichita State University
//Spring 2023
//code written by Joel Lewis
//adapted from code written by Everett Kernen in Fall 2021

#include <Servo.h> //this library is packaged with the arduino software and enables PWM output to the drive motor ESC
#include "PWM.hpp" //this library adds the ability to read the PWM RC reciever inputs with intterupts


/////////////PINS USED ON ARDUINO////////////////////////////////////////
  //either plug wires into the pins we've already written or write in the pins you're using
  //i.e. digital pin 10 --> 10
  //for analog, use prefix A, i.e. analog pin 5 --> A5
  
  const int JFWDBWDPIN = A1;  //pin number for forward and backwards input on the joystick
            //yellow wire
  const int JLEFTRIGHTPIN = A0;//pin number for left and right input on the joystick
            //green wire
  const int ACTUATORFEEDBACK = A2;//pin number for linear actuator feedback
            //blue wire
  const int MOTORESC = 10;   //pin number for ESC connecting to both drive motors
            //white wire
  const int ACTUATORLEFT = 5;  //pin number for actuator left signal
            //orange wire
  const int ACTUATORRIGHT = 6;  //pin number for actuator right signal
            //green wire

  //////////interrupt pins////////////////////////////////////////////
  //a standard arduino uno or nano can not be used for this as they only have 2 intteruptable pins from the ATmega328
  //an arduino pro micro is used since it has 5 intteruptable pins; any other ATmega32U4 based board can also support this
  //these pins are capable of reciving the PWM signals from the RC controller
  //without the added wait time from using pulseIn() on multiple pins

  const int RCTHROTTLEPIN = 7;  //pin number for throttle from RC controller
            //purple wire
  const int RCSTEERPIN = 3;  //pin number for steering from RC controller
            //yellow wire
  const int RCSTOPPIN = 2; //pin number for emergency stop from RC controller
            //blue wire

  ///////////trim potentiometer pin//////////////////////////
  const int TRIMPOTPIN = A3;

//CONSTANT VALUES
  //these values should already work; however you may need to change them
  //if joystick is initializing too quickly or too late
  //or if steering motor activates too often
  //or if the steering isn't centered after "returning to center"

  //if you do need to change these values, I recommend you use the Serial Monitor to check the values
  //as you turn the joystick, what values are you seeing and what values do you need to be seeing?
  //you can see the Serial Monitor by going to Tools->Serial Monitor

  //CONSTANT VALUES TO CHECK AGAINST JOYSTICK VALUES
    //joystick initialization values
    //change these if joystick initializes too quickly or too late
    //middle is ~520 or so

    //as mentioned below with the encoder values, we see JOYSTICKMIDDLE differently when the arduino is not connected to the computer
    //we started with 520, we moved to 630 (+110 again)
    //see lines 65-68 for more info
    const int JOYSTICKMIDDLE = 505;
    const int JOYSTICKDEADZONE = 15;
    const int JOYSTICKFULLLEFT = 1005;
    const int JOYSTICKFULLRIGHT = 0;
    // const int FWDVALUE = JOYSTICKMIDDLE - JOYSTICKDEADZONE;   //used to check "if joystick forward value is above FWDVALUE, go forward"
    // const int BWDVALUE = JOYSTICKMIDDLE + JOYSTICKDEADZONE;   //same as above, just if below BWDVALUE, go backwards
    // const int LEFTVALUE = JOYSTICKMIDDLE - JOYSTICKDEADZONE;  //same as above, just left
    // const int RIGHTVALUE = JOYSTICKMIDDLE + JOYSTICKDEADZONE; //same as above, just right


  //SPEED VALUES USED FOR STEERING
    //how fast the steering turns
    //for our motor controller, the arduino views it as a servo
    //as such, we use writeMicroseconds to control the speed
    //this is only relevant if you don't use the same motor controller
    //if you don't or you just want more information, visit https://www.arduino.cc/en/Reference/ServoWriteMicroseconds or the arduino main reference page
    //anyways, for our motor controller, full speed one way is 2500; full speed the other way is 500
    //1500 is middle (no turning at all)

    //if the joystick turns the wrong way, switch the right and left values
    ///////////for linear actuator steering position//////////////////
    const int FULLACTUATORLEFT = 55;
    const int FULLACTUATORRIGHT = 220;
    const int CENTERACTUATOR = (((FULLACTUATORRIGHT - FULLACTUATORLEFT) / 2) + FULLACTUATORLEFT);
    const int ACTUATORSPEED = 255;
    const int ACTUATORSTOP = 0;

    //////////for drive motor speed///////////////////
    const int FULLSPEEDBWD = 1100;
    const int FULLSPEEDFWD = 1850;
    const int ZEROSPEED = 1450;
    const int ESCSTOPBUFF = 100;

  ////////////standby values for RC controller inputs////////////////////////
    //This is used to transfer control of the car from the RC contorller to the joystick when the arduino is recieving PWM signals from the RC reciver,
    //but all the inputs on the RC controller are in their standby positions meaning the RC contoller user is not attempting to control the car,
    //and therefore it is safe for control to be transfered to the joystick
    //only when all three of the values recieved from the RC reciever are close their standby values, will control be transfered to the joystick
    const int STANDBYRCSTOP = 1670;
    const int STANDBYRCTHROTTLE = 1450;
    const int STANDBYRCSTEER = 1500;

    const int STANDBYBUFFER = 200;
    const int LOWSTANDBYRCSTOP = STANDBYRCSTOP - STANDBYBUFFER;
    const int HIGHSTANDBYRCSTOP = STANDBYRCSTOP + STANDBYBUFFER;
    const int LOWSTANDBYRCTHROTTLE = STANDBYRCTHROTTLE - STANDBYBUFFER;
    const int HIGHSTANDBYRCTHROTTLE = STANDBYRCTHROTTLE + STANDBYBUFFER;
    int LOWSTANDBYRCSTEER = STANDBYRCSTEER - STANDBYBUFFER;
    int HIGHSTANDBYRCSTEER = STANDBYRCSTEER + STANDBYBUFFER;

////////////////////default trim potentiometer value////////////////////////////
    const int CENTERTRIMPOT = 512;

//size of sample for averaging values from analog reads
int AVGSAMPLESIZE = 25;
int JLRSAMPLETOT;
int STRSAMPLETOT;
unsigned int RCLRSAMPLETOT;
int TRIMSAMPLETOT;

int AVGJLEFTRIGHTARR[50];
int AVGRCLEFTRIGHTARR[50];
int AVGSTEERARR[50];

int AVGTRIMPOTARR[50];

//INTEGER FOR TRIM POTENTIOMETER
int TRIMPOT;

//INTEGERS USED TO HOLD VALUES FROM JOYSTICK
int JFWDBWD;          
int JLEFTRIGHT;
int AVGJLEFTRIGHT;
int AVGRCLEFTRIGHT;

//INTEGER TO HOLD POSTION OF LINEAR ACTUATOR
int STEERPOS;
int AVGSTEERPOS;

int STEERPOSCENTER;

int STEERPOSBUFF = 15;
int TEMPBUFF = 30;

int RCSTEERPOS;

//INTEGER TO HOLD SPEED FOR ESC
int ESCSPEED;

//Drive Motor ESC, this is viewed by arduino as a servo
//servo PWM ranges from 1100 - 1900 microseconds. This is a different range
//and pulse width than the PWM seen from analogWrite().
Servo DRIVEESC;

//delcare PWM objects for reading PWM values from RC controller
PWM RCSTOP(RCSTOPPIN);
PWM RCTHROTTLE(RCTHROTTLEPIN);
PWM RCSTEER(RCSTEERPIN);

//-------------------FUNCTIONS START HERE--------------------------------

// put your setup code here, to run once:

  int count = 0;

void setup() {
  
  Serial.begin(9600);   //tells arduino how fast to interact with the Serial Monitor

  // setup inital average values for joystick steering and linear actuator feedback


  for(int i = 0; i < AVGSAMPLESIZE; i++) {
    AVGJLEFTRIGHTARR[i] = JOYSTICKMIDDLE; //default samples as center of joystick
    AVGRCLEFTRIGHTARR[i] = STANDBYRCSTEER; //default samples as center of actuator
    AVGSTEERARR[i] = CENTERACTUATOR; //default samples as center of actuator
    AVGTRIMPOTARR[i] = CENTERTRIMPOT;  //default samples as center of trim potentiometer
  }

  //calculate default average
  for(int i = 0; i < AVGSAMPLESIZE; i++) {
    JLRSAMPLETOT += AVGJLEFTRIGHTARR[i];
    RCLRSAMPLETOT += AVGRCLEFTRIGHTARR[i];
    STRSAMPLETOT += AVGSTEERARR[i];
    TRIMSAMPLETOT += AVGTRIMPOTARR[i];
  }
  JLEFTRIGHT = JLRSAMPLETOT/AVGSAMPLESIZE;
  STEERPOS = STRSAMPLETOT/AVGSAMPLESIZE;
  RCSTEERPOS = RCLRSAMPLETOT/AVGSAMPLESIZE;
  TRIMPOT = TRIMSAMPLETOT/AVGSAMPLESIZE;



//set input pins for arduino
  pinMode(JFWDBWDPIN, INPUT);
  pinMode(JLEFTRIGHTPIN, INPUT);
  pinMode(ACTUATORFEEDBACK, INPUT);
  pinMode(TRIMPOTPIN, INPUT);

//set output pins for arduino
  pinMode(ACTUATORLEFT, OUTPUT);
  pinMode(ACTUATORRIGHT, OUTPUT);

//setup PWM inputs
  RCSTOP.begin(true);
  RCTHROTTLE.begin(true);
  RCSTEER.begin(true);

//sets up Drive motor ESC control
  DRIVEESC.attach(MOTORESC);

// Drive motor initially set to zero speed
  DRIVEESC.writeMicroseconds(ZEROSPEED);
}

// put your main code here, to run repeatedly:
void loop() {

  
  
  //take in analog input to read what values the joystick is sending
  JFWDBWD = analogRead(JFWDBWDPIN);    


// ///////////////////////////////////////////////////////////////////

//   JLEFTRIGHT = analogRead(JLEFTRIGHTPIN);

//   //take in analog input from linear actuator for steering position
//   STEERPOS = analogRead(ACTUATORFEEDBACK);

// //////////////////////////////////////////////////////////////////


//shift old sample values one index down to make space for new value
//last value in the sample is overwritten
  for(int i = (AVGSAMPLESIZE - 1); i >= 1; i--) {
    AVGJLEFTRIGHTARR[i] = AVGJLEFTRIGHTARR[i - 1];
    AVGRCLEFTRIGHTARR[i] = AVGRCLEFTRIGHTARR[i - 1];
    AVGSTEERARR[i] = AVGSTEERARR[i - 1];
    AVGTRIMPOTARR[i] = AVGTRIMPOTARR[i - 1];
  }

  //update arrays with newest analog values
  AVGJLEFTRIGHTARR[0] = analogRead(JLEFTRIGHTPIN);
  AVGRCLEFTRIGHTARR[0]= RCSTEER.getValue();
  AVGSTEERARR[0] = analogRead(ACTUATORFEEDBACK);
  AVGTRIMPOTARR[0] = analogRead(TRIMPOTPIN);

  //calculate new averages
  JLRSAMPLETOT = 0;
  RCLRSAMPLETOT = 0;
  STRSAMPLETOT = 0;
  TRIMSAMPLETOT = 0;
  for(int i = 0; i < AVGSAMPLESIZE; i++) {
    JLRSAMPLETOT += AVGJLEFTRIGHTARR[i];
    RCLRSAMPLETOT += AVGRCLEFTRIGHTARR[i];
    STRSAMPLETOT += AVGSTEERARR[i];
    TRIMSAMPLETOT += AVGTRIMPOTARR[i];
  }
  JLEFTRIGHT = JLRSAMPLETOT/AVGSAMPLESIZE;
  STEERPOS = STRSAMPLETOT/AVGSAMPLESIZE;
  RCSTEERPOS = RCLRSAMPLETOT/AVGSAMPLESIZE;
  TRIMPOT = TRIMSAMPLETOT/AVGSAMPLESIZE;

  TRIMPOT = TRIMPOT - 500;

  //print out values to the serial monitor so that we can look at them

  if(count == 100) {
    count = 0;
    Serial.print("Joystick Yval: ");
    Serial.println(JFWDBWD);
    Serial.print("Joystick Xval: ");
    Serial.println(JLEFTRIGHT);
    Serial.print("Output Steering: ");
    Serial.println(STEERPOS);
    Serial.print("Output to ESC: ");
    Serial.println(ESCSPEED);
    Serial.print("RC Stop: ");
    Serial.println(RCSTOP.getValue());
    Serial.print("RC Throttle: ");
    Serial.println(RCTHROTTLE.getValue());
    Serial.print("RC Steer: ");
    Serial.println(RCSTEERPOS);
    Serial.print("Trim Pot: ");
    Serial.println(TRIMPOT);
    Serial.println();
  }
  count++;


  //if statement checks if RC controller has an input change from its default position,
  //if it detects a change in input the RC controller has control
  //if no change is detected, the joystick has control

  ///////////////////////////////check for RC emergency stop///////////////////////////////////////////
  
  if((RCSTOP.getValue() < LOWSTANDBYRCSTOP) || (RCSTOP.getValue() > HIGHSTANDBYRCSTOP)) {
    DRIVEESC.writeMicroseconds(ZEROSPEED);    //stop drive motors
    analogWrite(ACTUATORLEFT, ACTUATORSTOP);  //stop steer actuator
    analogWrite(ACTUATORRIGHT, ACTUATORSTOP); //stop steer actuator
  }
  else {

    ////////////////////////////////////RC control///////////////////////////////////////////////////////    
    ESCSPEED = RCTHROTTLE.getValue(); //assigns RC throttle value to ESCSPEED
    if((ESCSPEED < (ZEROSPEED - ESCSTOPBUFF)) || (ESCSPEED > (ZEROSPEED + ESCSTOPBUFF)) || (RCSTEER.getValue() < LOWSTANDBYRCSTEER) || (RCSTEER.getValue() > HIGHSTANDBYRCSTEER)) {  //checks if RC Throttle or RC Steering is being used
      
      if((ESCSPEED < (ZEROSPEED - ESCSTOPBUFF)) || (ESCSPEED > (ZEROSPEED + ESCSTOPBUFF))) {//drive motor control
        DRIVEESC.writeMicroseconds(ESCSPEED);   //sends RC throttle value to drive motor
      }
      else{ //drive motor off
        DRIVEESC.writeMicroseconds(ZEROSPEED);
      }

      RCSTEERPOS = map(RCSTEERPOS, FULLSPEEDBWD + 75, FULLSPEEDFWD - 25, FULLACTUATORLEFT, FULLACTUATORRIGHT); //assigns RC steer value to RCSTEERPOS

      // if((RCSTEERPOS < CENTERACTUATOR - STEERPOSBUFF) && (STEERPOS > FULLACTUATORLEFT + 10)) {
      //   analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      //   analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
      // }
      // else if((RCSTEERPOS > CENTERACTUATOR + STEERPOSBUFF) && (STEERPOS < FULLACTUATORRIGHT - 10)) {
      //   analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      //   analogWrite(ACTUATORLEFT, ACTUATORSPEED);
      // } 
      // else {
      //   analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      //   analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      // }


      //hysterysis protection
      LOWSTANDBYRCSTEER = STANDBYRCSTEER - STANDBYBUFFER/2;
      HIGHSTANDBYRCSTEER = STANDBYRCSTEER + STANDBYBUFFER/2;
   

      if ((STEERPOS >= (RCSTEERPOS - (STEERPOSBUFF))) && (STEERPOS <= (RCSTEERPOS + (STEERPOSBUFF)))) {
        analogWrite(ACTUATORLEFT, ACTUATORSTOP);
        analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      }
      else if(STEERPOS > (RCSTEERPOS + (STEERPOSBUFF*2))) {
        analogWrite(ACTUATORLEFT, ACTUATORSTOP);
        analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
        delay(5);
      }
      else if(STEERPOS < (RCSTEERPOS - (STEERPOSBUFF*2))) {
        analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
        analogWrite(ACTUATORLEFT, ACTUATORSPEED);
        delay(5);
      }
    }

    /////////////////////////joystick takes over control////////////////////////////////////////////////
    else {  
      //resest hysterysis
      LOWSTANDBYRCSTEER = STANDBYRCSTEER - STANDBYBUFFER;
      HIGHSTANDBYRCSTEER = STANDBYRCSTEER + STANDBYBUFFER;
      ////////////////joystick drive control/////////////////////////////////////
      ESCSPEED = map(JFWDBWD, 0, 1023, FULLSPEEDBWD, FULLSPEEDFWD);
      if((ESCSPEED < (ZEROSPEED - ESCSTOPBUFF)) || (ESCSPEED > (ZEROSPEED + ESCSTOPBUFF))) {//drive motor control
        DRIVEESC.writeMicroseconds(ESCSPEED);   //sends joystick throttle y value to drive motor
      }
      else{ //drive motor off
        DRIVEESC.writeMicroseconds(ZEROSPEED);
      }

      /////////////joystick steering control///////////////////////////////////
      // STEERPOSCENTER = 127;
      // if (STEERPOS < 80){
      //   analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      //   analogWrite(ACTUATORLEFT, ACTUATORSPEED);
        
      // }
      // else if (STEERPOS > 210){  //stop linear actuator when actuator position is equal to joystick position
      //   analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      //   analogWrite(ACTUATORRIGHT, ACTUATORSPEED);

      // }
      // else {
      //   JLEFTRIGHT = map(JLEFTRIGHT, 0, 1023, 0, 255);
      //   if((JLEFTRIGHT + (STANDBYBUFFER/8)) < (STEERPOSCENTER)) {    //drive linear actuator left
      //     analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      //     analogWrite(ACTUATORLEFT, ACTUATORSPEED);
      //   } 
      //   else if((JLEFTRIGHT - (STANDBYBUFFER/8)) > (STEERPOSCENTER)) {  //drive linear actuator right
      //     analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      //     analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
      //   } 
      //   else {
      //     analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      //     analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      //   }
      // }


      JLEFTRIGHT = map(JLEFTRIGHT + TRIMPOT/2, JOYSTICKFULLLEFT, JOYSTICKFULLRIGHT, FULLACTUATORLEFT, FULLACTUATORRIGHT);

      if ((STEERPOS >= (JLEFTRIGHT - STEERPOSBUFF)) && (STEERPOS <= (JLEFTRIGHT + STEERPOSBUFF))) {
        analogWrite(ACTUATORLEFT, ACTUATORSTOP);
        analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      }
      else if(STEERPOS > JLEFTRIGHT + STEERPOSBUFF) {
        analogWrite(ACTUATORLEFT, ACTUATORSTOP);
        analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
      }
      else if(STEERPOS < JLEFTRIGHT - STEERPOSBUFF) {
        analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
        analogWrite(ACTUATORLEFT, ACTUATORSPEED);
      }
      delay(5);

    }

  }

}


  /*

  //////////////////////check for RC steer input//////////////////////////////////
  else if((RCSTEER.getValue() < LOWSTANDBYRCSTEER) || (RCSTEER.getValue() > HIGHSTANDBYRCSTEER)) {
    if(JLEFTRIGHT < STEERPOS) {    //drive linear actuator left
      analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      analogWrite(ACTUATORLEFT, ACTUATORSPEED);
    } 
    else if(JLEFTRIGHT > STEERPOS) {  //drive linear actuator right
      analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
    } 
    else {  //stop linear actuator when actuator position is equal to joystick position
      analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
    }
  }
*/
  /////////if no controls from RC reciever are changed, hand off car control to the joystick/////////////////////
  /*else { 
    //////////////////////joystick steering control//////////////////////////////////
    if(JLEFTRIGHT < STEERPOS) {    //drive linear actuator left
      analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
      analogWrite(ACTUATORLEFT, ACTUATORSPEED);
    } 
    else if(JLEFTRIGHT > STEERPOS) {  //drive linear actuator right
      analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      analogWrite(ACTUATORRIGHT, ACTUATORSPEED);
    } 
    else {  //stop linear actuator when actuator position is equal to joystick position
      analogWrite(ACTUATORLEFT, ACTUATORSTOP);
      analogWrite(ACTUATORRIGHT, ACTUATORSTOP);
    }
    
    /////////////////////joystick drive control//////////////////////////////////////
    //maps joystick forward/backward position to a value that can be interpereted by DRIVEESC with writeMicroseconds()
    ESCSPEED = map(JFWDBWD, 0, 1023, FULLSPEEDBWD, FULLSPEEDFWD);

    if((ESCSPEED < (ZEROSPEED - ESCSTOPBUFF)) || (ESCSPEED > (ZEROSPEED + ESCSTOPBUFF))) {
      DRIVEESC.writeMicroseconds(ESCSPEED);   // writes the current speed of the motor if the joystick is outside of the stop buffer, this write includes forwards and backwards direction
    }
    else{ //drive motor off
      DRIVEESC.writeMicroseconds(ZEROSPEED);
    }
  }
  */
//}