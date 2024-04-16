//GoBabyGo Wichita State University
//Spring 2024
//project done by Everett Kernen, Joel Lewis, and Jacksn Truitt

//It should be noted this code uses a custom PCB to connect wires together. See details for that in the README, if I remember

#include <Servo.h> // enables PWM output to the drive and steer motors

//TODO:
//test (obviously)
//set up emergency stop func


//PINS (ordered clockwise around the arduino nano every)
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
int JOY_MOTOR_MIDDLE = 0;
int JOY_STEER_MIDDLE = 0;
const int JOY_DEADZONE = 130;         //how far away from center do you need to push the joystick before it's initialized?
int RC_SPEED_MIDDLE = 50;
int RC_STEERING_MIDDLE = 50;
int RC_STOP_MIDDLE = 50;
const int RC_DEADZONE = 25;

const int motor_maxDistanceFromZero = 400; // distance from fwdSpeed and bwdSpeed, used for speed controller
int motor_bwdSpeed = 1100;
const int motor_zeroSpeed = 1500;
int motor_fwdSpeed = 1900;
const int motor_brakeBuffer = 100;

const int steering_distanceFromZero = 500;
int steering_left = 1250;
int steering_middle = 1500;
int steering_right = 1750;



//VARIABLES
int joy_motorValue;
int joy_steerValue;
int speedValue;

//Motor Controller (viewed by arduino as a servo), see https://www.arduino.cc/reference/en/libraries/servo/writemicroseconds/ for how we are controlling the "Servos"
Servo MOTOR;
Servo STEER;


// see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ for info on interrupts, Interrupt Service Routines (ISRs), etc.
class PWM {
  int pin;
  bool prevState;
  unsigned long time;
  int value = -1; // given in terms of a duty cycle percentage
  unsigned int timeLow = 0;
  unsigned int timeHigh = 0;

  public:
    PWM(int pwm_pin) {
      pin = pwm_pin;
    }
    void start() {
      prevState = digitalRead(pin);
      time = micros();
      pinMode(pin, INPUT);
    }

    int GetValue() { return (value == -1 ? 50 : value); }

    void InterruptServiceRoutine() {
      unsigned long currTime = micros();
      bool currState = digitalRead(pin);
      if (currState != prevState) {
        if (currState) { // switched from low to high
          timeLow = currTime - time;
          time = currTime;
          value = (int)((timeHigh / (timeLow + timeHigh)) * 100);
            // value = percentage of time high
        } 
        else { // switched from high to low
          timeHigh = currTime - time;
          time = currTime;
        }
        prevState = currState;
      }
    }
};

//PWM class allows us to read PWM signals from the RC receiver using interrupts instead of pulseIn which can be slow
PWM rc_motor(RC_SPEED);
PWM rc_steer(RC_STEERING);
PWM rc_stop(RC_EMERGENCYSTOP);

// -------------------FUNCTIONS START HERE//------------------- //

// put your setup code here, to run once:
void setup() {

  Serial.begin(9600);   //tells arduino how fast to interact with the Serial Monitor

  //set input and output pins for arduino
  pinMode(JOYSTICKSWITCH, INPUT);
  pinMode(SPEEDCONTROLLER, INPUT);
  pinMode(JOYHORI, INPUT);
  pinMode(JOYVERT, INPUT);
  
  rc_motor.start();
  attachInterrupt(digitalPinToInterrupt(RC_SPEED), RCMotorInterrupt, CHANGE);
  rc_steer.start();
  attachInterrupt(digitalPinToInterrupt(RC_STEERING), RCSteerInterrupt, CHANGE);
  rc_stop.start();
  attachInterrupt(digitalPinToInterrupt(RC_EMERGENCYSTOP), RCStopInterrupt, CHANGE);
  
  MOTOR.attach(MOTOR_output);
  STEER.attach(STEERING_output);

  MOTOR.writeMicroseconds(motor_zeroSpeed);
  STEER.writeMicroseconds(steering_middle);

  setConstants();
}

// put your main code here, to run repeatedly:
void loop() {

  //take in analog input to read what values the joystick is sending
  joy_motorValue = analogRead(JOYVERT);
  joy_steerValue = analogRead(JOYHORI);

  //read speed controller and set speed values
  SetSpeed();


  //print out values to the serial monitor so that we can look at them
  /*
  Serial.print("Joy motor:          ");
  Serial.println(joy_motorValue);
  Serial.print("Joy steer:          ");
  Serial.println(joy_steerValue);
  Serial.print("RC motor:           ");
  Serial.println(rc_motor.GetValue());
  Serial.print("RC steer:           ");
  Serial.println(rc_steer.GetValue());
  Serial.print("RC Emergency Stop:  ");
  Serial.println(rc_stop.GetValue());
  Serial.println();
  */

  // TEST FOR EMERGENCY STOP
  if (false && (rc_stop.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_stop.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE)) {
    EmergencyStop();
  }

  // SEND SIGNAL TO MOTOR
  if (false && (rc_motor.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_motor.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE)) { // prioritize the rc controller
    MOTOR.writeMicroseconds(rc_motor.GetValue());
  }
  else if (joy_motorValue > JOY_MOTOR_MIDDLE + JOY_DEADZONE ||
            joy_motorValue < JOY_MOTOR_MIDDLE - JOY_DEADZONE) {
      int speed = map(joy_motorValue, 0, 1023, motor_bwdSpeed, motor_fwdSpeed);
      MOTOR.writeMicroseconds(speed);
  }
  else {
    MOTOR.writeMicroseconds(motor_zeroSpeed);
  }


  // SEND SIGNAL TO STEERING
  if (false && (rc_steer.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_steer.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE)) { // prioritize the rc controller
    STEER.writeMicroseconds(rc_steer.GetValue());
  }
  else if (joy_steerValue > JOY_STEER_MIDDLE + JOY_DEADZONE ||
            joy_steerValue < JOY_STEER_MIDDLE - JOY_DEADZONE) {
      int steer = map(joy_steerValue, 0, 1023, steering_left, steering_right);
      STEER.writeMicroseconds(steer);
  }
  else {
    STEER.writeMicroseconds(steering_middle);
  }

}

// ----------- HELPER FUNCTIONS ----------- //

void setConstants() {
  JOY_MOTOR_MIDDLE = analogRead(JOYVERT);
  JOY_STEER_MIDDLE = analogRead(JOYHORI);
}

void SetSpeed() {
  speedValue = analogRead(SPEEDCONTROLLER);
  speedValue /= 128; // cut down to values between 0 and 7, inclusive
  speedValue++; // 1 to 8
  int distFromZero = (int)(motor_maxDistanceFromZero / 3) + (int)(motor_maxDistanceFromZero / 12 * speedValue);
    //distFromZero is split into 8 sections in the top two thirds of motor_maxDistanceFromZero
  motor_fwdSpeed = motor_zeroSpeed + distFromZero;
  motor_bwdSpeed = motor_zeroSpeed - distFromZero;

  motor_fwdSpeed = 1900;
  motor_bwdSpeed = 1100;
}

void EmergencyStop() {
  // does rc send stop consistently? or just once?
  // if consistently, wait for release of button
  //while (rc_stop);
}

void RCMotorInterrupt() { rc_motor.InterruptServiceRoutine(); }
void RCSteerInterrupt() { rc_steer.InterruptServiceRoutine(); }
void RCStopInterrupt() { rc_stop.InterruptServiceRoutine(); }
