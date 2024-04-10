//GoBabyGo Wichita State University
//Spring 2024
//code written by Everett Kernen
//electronics done by Jackson Truitt and Everett Kernen
//if you have any questions, try not to; I've spent too much time commenting extensively
//but if you do still have questions, email me at ekernen2@gmail.com with the subject line reading "GoBabyGo" along with whatever else
//I'll try to respond in a semi-timely manner

//It should be noted this code uses a custom PCB to connect wires together. See details for that in the README, if I remember

#include <Servo.h>

//TODO:
//test (obviously)
//get correct constants
//take correct rc inputs
//modify those inputs to microsecond outputs
//set up setJoystick func
//set up emergency stop func


//PINS
const int JOYSTICKSWITCH = 12;
const int RC_SPEED = 10; //PWM
const int RC_STEERING = 9; //PWM
const int RC_EMERGENCYSTOP = 6; //PWM
const int STEERING_output = 5; //PWM - this controls either the linear actuator or servo, depending on which is selected to steer the car
const int MOTOR_output = 3; //PWM
const int SPEEDCONTROLLER = A0;
const int JOYHORI = A2;
const int JOYVERT = A4;

//CONSTANT VALUES
const int JOY_MOTOR_MIDDLE = 0;
const int JOY_STEER_MIDDLE = 0;
const int JOY_DEADZONE = 250;         //how far away from center do you need to push the joystick before it's initialized?

const int motor_bwdSpeed = 500;
const int motor_zeroSpeed = 1500;
const int motor_fwdSpeed = 2500;
const int steering_left = 500;
const int steering_middle = 1500;
const int steering_right = 2500;



//VARIABLES
int joy_motorvalue;
int joy_steervalue;
int rc_motor;
int rc_steer;
int rc_stop;

//Motor Controller (viewed by arduino as a servo)
Servo MOTOR
Servo STEER;

//-------------------FUNCTIONS START HERE--------------------------------

// put your setup code here, to run once:
void setup() {

  Serial.begin(9600);   //tells arduino how fast to interact with the Serial Monitor

  //set input and output pins for arduino
  pinMode(JOYSTICKSWITCH, INPUT);
  pinMode(RC_SPEED, INPUT);
  pinMode(RC_STEERING, INPUT);
  pinMode(RC_EMERGENCYSTOP, INPUT);
  pinMode(SPEEDCONTROLLER, INPUT);
  pinMode(JOYHORI, INPUT);
  pinMode(JOYVERT, INPUT);
  
  MOTOR.attach(MOTOR_output)
  STEER.attach(STEERING_output);

  MOTOR.writeMicroseconds(motor_zerospeed);
  STEER.writeMicroseconds(steering_middle);

  setJoystick();
}

// put your main code here, to run repeatedly:
void loop() {

  //take in analog input to read what values the joystick and RC controller are sending
  joy_motorvalue = analogRead(JOYVERT);
  joy_steervalue = analogRead(JOYHORI);
  rc_motor = GetPWM(RC_MOTOR);
  rc_steer = GetPWM(RC_STEER);
  rc_stop = GetPWM(RC_EMERGENCYSTOP);


  //print out values to the serial monitor so that we can look at them
  Serial.print("Joy motor:          ");
  Serial.println(joy_motorvalue);
  Serial.print("Joy steer:          ");
  Serial.println(joy_steervalue);
  Serial.print("RC motor:           ");
  Serial.println(rc_motor);
  Serial.print("RC steer:           ");
  Serial.println(rc_steer);
  Serial.print("RC Emergency Stop:  ");
  Serial.println(rc_stop);
  Serial.println();

  // TEST FOR EMERGENCY STOP
  if (rc_stop != 0) {
    EmergencyStop();
  }

  // SEND SIGNAL TO MOTOR
  if (rc_motor != 0) { // prioritize the rc controller
    MOTOR.writeMicroseconds(rc_motor);
  }
  else {
    if (joy_motorvalue > JOY_MOTORMIDDLE + JOY_DEADZONE) {
      MOTOR.writeMicroseconds(motor_fwdSpeed);
    }
    else if (joy_motorvalue < JOY_MOTORMIDDLE - JOY_DEADZONE) {
      MOTOR.writeMicroseconds(motor_bwdSpeed);
    }
    else {
      MOTOR.writeMicroseconds(motor_zeroSpeed);
    }
  }
  
  // SEND SIGNAL TO STEERING
  if (rc_steer != 0) { // prioritize the rc controller
    STEER.writeMicroseconds(rc_steer);
  }
  else {
    if (joy_motorvalue > JOY_MOTORMIDDLE + JOY_DEADZONE) {
      STEER.writeMicroseconds(steering_right);
    }
    else if (joy_motorvalue < JOY_MOTORMIDDLE - JOY_DEADZONE) {
      STEER.writeMicroseconds(steering_left);
    }
    else {
      STEER.writeMicroseconds(steering_middle);
    }
  }
}

  void setJoystick(){
    //is joystickswitch flipped?
  }

  byte GetPWM(byte pin)
  {
    unsigned long highTime = pulseIn(pin, HIGH, 50000UL);  // 50 millisecond timeout
    unsigned long lowTime = pulseIn(pin, LOW, 50000UL);  // 50 millisecond timeout

    // pulseIn() returns zero on timeout
    if (highTime == 0 || lowTime == 0)
      return digitalRead(pin) ? 100 : 0;  // HIGH == 100%,  LOW = 0%

    return (100 * highTime) / (highTime + lowTime);  // highTime as percentage of total cycle time
  }

  void EmergencyStop() {
    // does rc send stop consistently? or just once?
    // if consistently, wait for release of button
    while (rc_stop);
  }
