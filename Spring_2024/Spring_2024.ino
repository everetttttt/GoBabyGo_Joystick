//GoBabyGo Wichita State University
//Spring 2024
//code written by Everett Kernen
//electronics done by Jackson Truitt and Everett Kernen
//if you have any questions, try not to; I've spent too much time commenting extensively
//but if you do still have questions, email me at ekernen2@gmail.com with the subject line reading "GoBabyGo" along with whatever else
//I'll try to respond in a semi-timely manner

#include <Servo.h>


//PINS USED IN ARDUINO
//either plug wires into the pins we've already written or write in the pins you're using
//a number is equal to its digital counterpart, i.e. digital pin 10 --> 10
//for analog, use prefix A, i.e. analog pin 5 --> A5
const int SPEEDCONTROLLERPOSITIVE = 12;
const int SPEEDCONTROLLERPINin = A0; //TBD
const int SPEEDCONTROLLERNEGATIVE = 11;
const int RCTRANSMITTERONEin = 10; //PWM channel
const int RCTRANSMITTERTWOin = 6; //PWM channel
const int RCTRANSMITTERTHREEin = 5; //PWM channel
const int LINEARACTUATORSIGNALout = 3; //PWM channel
const int JOYSTICKHORIZONTAL = A0;
const int JOYSTICKVERTICAL = A2;




const int JFWDBWDPIN = A2;  //pin number for forward and backwards input on the joystick
const int JSIDEWAYSPIN = A4;//pin number for left and right input on the joystick
const int GAS = 2;         //pin number for the Gas relay input
const int FWD = 4;          //forward relay
const int BWD = 6;          //backward relay
//IF YOU DO NOT IMPLEMENT THESE CONTROLLER INPUTS, just connect these pins to ground
const int RIGHTMOTOR = 10;   //signal in from the remote that you are turning right
const int LEFTMOTOR = 12;    //signal in from the remote that you are turning left
const int ENCODER = A6;     //encoder signal pin (the encoder is what measures where the steering shaft is rotationally)
const int STEERMOTOR = 8;   //motor controller output pin

//CONSTANT VALUES
//these values should already work; however you may need to change them
//if joystick is initializing too quickly or too late
//if steering motor activates too often
//if the steering isn't centered after "returning to center"

/*if you do need to change these values, I recommend you use the Serial Monitor to check the values.
  As you turn the joystick, what values are you seeing and what values do you want to be seeing?
  you can see the Serial Monitor by going to Tools->Serial Monitor */

//CONSTANT VALUES TO CHECK AGAINST JOYSTICK VALUES
//joystick initialization values
//change these if joystick initializes too quickly or too late
//middle is ~520 or so

/*as mentioned below with the encoder values, we see JOYSTICKMIDDLE differently when the arduino is not connected to the computer
  we started with 520, we moved to 630 (+110 again)
  see lines 67-70 for more info*/
const int JOYSTICKUPDOWNMIDDLE = 600;       //middle value for moving joystic up and down
const int JOYSTICKLEFTRIGHTMIDDLE = 730;    //not entirely sure why the values are different for fwd/bwd and left/right, probably improper manufacturing
const int JOYSTICKDEADZONE = 250;           //how far away from center do you need to push the joystick before it's initialized?
const int FWDVALUE = JOYSTICKUPDOWNMIDDLE - JOYSTICKDEADZONE;   //used to check "if joystick forward value is above FWDVALUE, go forward"
const int BWDVALUE = JOYSTICKUPDOWNMIDDLE + JOYSTICKDEADZONE;   //same as above, just if below BWDVALUE, go backwards
const int LEFTVALUE = JOYSTICKLEFTRIGHTMIDDLE - JOYSTICKDEADZONE;  //same as above, just left
const int RIGHTVALUE = JOYSTICKLEFTRIGHTMIDDLE + JOYSTICKDEADZONE; //same as above, just right
//IF JOYSTICK CAUSES CAR TO DO THE OPPOSITE THING ITS SUPPOSED TO
//you probably oriented the joystick differently than how we did, try modifying lines 45-51 by switching + with -
//and modify lines 165, 171, 210, and 220
//line numbers may not be entirely accurate, they refer to the if statements
//e.g. switch "if (JFWDBWD > FWDVALUE)" to "if (JFWDBWD < FWDVALUE)"

//CONSTANT VALUES TO CHECK AGAINST ENCODER VALUE
/*steering centering values
  change these if steering doesn't return to center properly
  values go from 0-1023, clockwise turning moves the number up
  you WILL need to modify ENCODERCENTER to begin with, it depends on how you install the encoder
  make sure you install the encoder such that when you turn, it doesn't loop around from, for example, 800 to 200 by going 800->1023 then 0->200
  that will view to you as the edges are 800 and 200, realign the encoder such that the middle and both edges are all within one loop of 0-1023*/

/*when you unplug the usb, the steering goes off center
  we think this is just due to processing differences between computer and arduino
  the processing differences are irrelevant to you as the user, all you need to know is
  you'll have to play with ENCODERCENTER to fix this issue. We went from 590 to 700*/
const int ENCODERCENTER = 680;
//modify these two if the center range is too broad or too narrow for your taste
/*the logic used during use is "if the steering is outside of the ENCODERCOARSE range (and joystick isn't pushed left or right),
  go full speed towards center. Once you're inside of the ENCODERCOARSE range, go half speed until in ENCODERFINE range. Once inside the fine range,
  you're close enough to center" */
//The steering rack is fairly loose so a tighter ENCODERFINE didn't do anything for us; play with it if you'd like though
const int ENCODERFINE = 30;
const int ENCODERCOARSE = 70;
//Encoder right and left are the limits of where the wheels are turned all the way one direction
//once it reaches this end, we send a message to the steer motor to not turn anymore
const int ENCODERRIGHT = ENCODERCENTER - 50;
const int ENCODERLEFT = ENCODERCENTER + 50;
//IF THE ENCODER IS INSTALLED BACKWARDS OR THE STEERING RETURNS TO CENTER THE WRONG WAY
//play with lines 81, 82; 232, 234, 243, and 245 by switching + with - and < with >
//same as line 56, line numbers may not be entirely accurate. They refer to the if statements


//SPEED VALUES USED FOR STEERING
/*how fast the steering turns
  for our motor controller, we set the arduino to view it as a servo
  as such, we use writeMicroseconds to control the speed
  this is only relevant if you don't use the same motor controller or you set it up differently
  if you don't or you just want more information, visit https://www.arduino.cc/en/Reference/ServoWriteMicroseconds or the arduino main reference page
  anyways, for our motor controller, full speed one way is 2500; full speed the other way is 500
  1500 is middle (no turning at all)*/

//if the joystick turns the wrong way, switch the right and left values
const int FULLSPEEDRIGHT = 750;
const int PARTSPEEDRIGHT = 1150;
const int ZEROSPEED = 1500;
const int PARTSPEEDLEFT = 1850;
const int FULLSPEEDLEFT = 2250;

//-----DO NOT MODIFY PAST THIS LINE UNLESS YOU KNOW WHAT YOU'RE DOING-----


//INTEGERS USED TO READ VALUES FROM JOYSTICK
int JFWDBWD;
int JLEFTRIGHT;

//INTEGER TO READ VALUE FROM ENCODER (piece attached to steering shaft)
int ENCODERVAL;

//Motor Controller (viewed by arduino as a servo)
Servo STEER;

//-------------------FUNCTIONS START HERE--------------------------------

// put your setup code here, to run once:
void setup() {

  Serial.begin(9600);   //tells arduino how fast to interact with the Serial Monitor

  //set input and output pins for arduino
  pinMode(JFWDBWDPIN, INPUT);
  pinMode(JSIDEWAYSPIN, INPUT);
  pinMode(ENCODER, INPUT);
  pinMode(FWD, OUTPUT);
  pinMode(BWD, OUTPUT);
  pinMode(GAS, OUTPUT);
  pinMode(RIGHTMOTOR, INPUT);
  pinMode(LEFTMOTOR, INPUT);
  STEER.attach(STEERMOTOR, 500, 2500);

  //safekeeping measure so relays are set to the car not running to the races on startup
  //also, the steering motor is set to not do anything
  digitalWrite(GAS, LOW);
  digitalWrite(FWD, LOW);
  digitalWrite(BWD, LOW);
  STEER.writeMicroseconds(ZEROSPEED);
}

// put your main code here, to run repeatedly:
void loop() {

  //take in analog input to read what values the joystick is sending
  JFWDBWD = analogRead(JFWDBWDPIN);
  JLEFTRIGHT = analogRead(JSIDEWAYSPIN);
  ENCODERVAL = analogRead(ENCODER);

  //print out values to the serial monitor so that we can look at them
  Serial.print("Yval: ");
  Serial.println(JFWDBWD);
  Serial.print("Xval: ");
  Serial.println(JLEFTRIGHT);
  Serial.print("Steering: ");
  Serial.println(ENCODERVAL);
  Serial.println();
  Serial.print("Left Value:        ");
  Serial.println(digitalRead(LEFTMOTOR));
  Serial.print("Right Value:                          ");
  Serial.println(digitalRead(RIGHTMOTOR));


  if (JFWDBWD < FWDVALUE) {
    //if joystick is forward, go forward
    digitalWrite(FWD, HIGH);
    digitalWrite(GAS, HIGH);
    digitalWrite(BWD, LOW);
  }
  else if (JFWDBWD > BWDVALUE) {
    //if joystick is backward, go backwarde
    digitalWrite(BWD, HIGH);
    digitalWrite(GAS, HIGH);
    digitalWrite(FWD, LOW);
  }
  else {
    //"if joystick y value is centered"
    //set relays so nothing is on
    digitalWrite(FWD, LOW);
    digitalWrite(BWD, LOW);
    digitalWrite(GAS, LOW);
  }

  /*we test for digitalRead before we test for joystick values to prioritize the parent remote
    this does add complexity to the build process, as it requires you add the ability to read the values
    the computer is trying to send to the motor. We did this via two voltage regulators (12V down to 5V) and two diodes*/
  if (digitalRead(RIGHTMOTOR) == HIGH) {
    //turn right
    if (ENCODERVAL < ENCODERRIGHT) {
      //if it's already all the way right, no need to turn further and strain the motor/gears
      STEER.writeMicroseconds(ZEROSPEED);
    }
    else {
      STEER.writeMicroseconds(FULLSPEEDRIGHT);
    }
  }
  else if (digitalRead(LEFTMOTOR) == HIGH) {
    //turn left
    if (ENCODERVAL > ENCODERLEFT) {
      //if it's already all the way left, no need to turn further and strain the motor/gears
      STEER.writeMicroseconds(ZEROSPEED);
    }
    else {
      STEER.writeMicroseconds(FULLSPEEDLEFT);
    }
  }


  else if (JLEFTRIGHT > RIGHTVALUE) {
    //turn right
    if (ENCODERVAL < ENCODERRIGHT) {
      //if it's already all the way right, no need to turn further and strain the motor/gears
      STEER.writeMicroseconds(ZEROSPEED);
    }
    else {
      STEER.writeMicroseconds(FULLSPEEDRIGHT);
    }
  }
  else if (JLEFTRIGHT < LEFTVALUE) {
    //turn left
    if (ENCODERVAL > ENCODERLEFT) {
      //if it's already all the way left, no need to turn further and strain the motor/gears
      STEER.writeMicroseconds(ZEROSPEED);
    }
    else {
      STEER.writeMicroseconds(FULLSPEEDLEFT);
    }
  }
  else {
    //"if joystick x value is centered"
    if (ENCODERVAL > (ENCODERCENTER + ENCODERFINE)) {
      //if the steering is turned to the left
      if (ENCODERVAL > (ENCODERCENTER + ENCODERCOARSE)) {
        //if it's turned all the way to the left, turn right full speed
        STEER.writeMicroseconds(PARTSPEEDRIGHT);
      }
      else {
        //if it's turned only a little to the left, turn right at a lower speed
        STEER.writeMicroseconds(PARTSPEEDRIGHT);
      }
    }
    else if (ENCODERVAL < (ENCODERCENTER - ENCODERFINE)) {
      //if the steering is turned to the right
      if (ENCODERVAL < (ENCODERCENTER - ENCODERCOARSE)) {
        //if it's turned all the way to the right, turn left full speed
        STEER.writeMicroseconds(PARTSPEEDLEFT);
      }
      else {
        //if it's turned only a little to the right, turn left at a lower speed
        STEER.writeMicroseconds(PARTSPEEDLEFT);
      }
    }
    else {
      //if the joystick is centered and so is the steering, do nothing
      STEER.writeMicroseconds(ZEROSPEED);
    }
  }

}
