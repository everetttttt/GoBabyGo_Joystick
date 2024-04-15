//GoBabyGo Wichita State University
//Spring 2024
//project done by Everett Kernen, Joel Lewis, and Jacksn Truitt

//It should be noted this code uses a custom PCB to connect wires together. See details for that in the README, if I remember

#include <Servo.h> // enables PWM output to the drive and steer motors

//TODO:
//test (obviously)
//get correct constants
//take correct rc inputs
//modify those inputs to microsecond outputs
//set up setJoystick func
//set up emergency stop func
//add speed controller testing


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
const int JOY_DEADZONE = 250;         //how far away from center do you need to push the joystick before it's initialized?
int RC_SPEED_MIDDLE = 0;
int RC_STEERING_MIDDLE = 0;
int RC_STOP_MIDDLE = 0;
const int RC_DEADZONE = 200;


const int motor_bwdSpeed = 1100;
const int motor_zeroSpeed = 1450;
const int motor_fwdSpeed = 1850;
const int motor_brakeBuffer = 100;

const int steering_left = 500;
const int steering_middle = 1500;
const int steering_right = 2500;



//VARIABLES
int joy_motorvalue;
int joy_steervalue;

//Motor Controller (viewed by arduino as a servo)
Servo MOTOR;
Servo STEER;


// see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ for info on interrupts, Interrupt Service Routines (ISRs), etc.
class PWM {
  int pin;
  bool prev_state;
  unsigned long time;
  int value;
  unsigned int time_low = 0;
  unsigned int time_high = 0;

  public:
    PWM(int pwm_pin) {
      pin = pwm_pin;
    }
    void start() {
      prev_state = digitalRead(pin);
      time = micros();
      pinMode(pin, INPUT);
      attachInterrupt(digitalPinToInterrupt(pin), InterruptServiceRoutine, CHANGE);
    }

    int GetValue() { return value; }

    void InterruptServiceRoutine() {
      unsigned long curr_time = micros();
      bool curr_state = digitalRead(pin);
      if (curr_state != prev_state) {
        if (curr_state) { // switched from low to high
          time_low = curr_time - time;
          time = curr_time;
          value = (int)((time_high / (time_low + time_high)) * 100);
            // save percentage of time high to value
        } 
        else { // switched from high to low
          time_high = curr_time - time;
          time = curr_time;
        }
        prev_state = curr_state;
      }
    }
};

//PWM class allows us to read PWM signals from the RC receiver using interrupts instead of pulseIn which can be slow
PWM rc_motor(RC_SPEED);
PWM rc_steer(RC_STEERING);
PWM rc_stop(RC_EMERGENCYSTOP);

//-------------------FUNCTIONS START HERE--------------------------------

// put your setup code here, to run once:
void setup() {

  Serial.begin(9600);   //tells arduino how fast to interact with the Serial Monitor

  //set input and output pins for arduino
  pinMode(JOYSTICKSWITCH, INPUT);
  pinMode(SPEEDCONTROLLER, INPUT);
  pinMode(JOYHORI, INPUT);
  pinMode(JOYVERT, INPUT);
  
  rc_motor.start();
  rc_steer.start();
  rc_stop.start();
  
  MOTOR.attach(MOTOR_output);
  STEER.attach(STEERING_output);

  MOTOR.writeMicroseconds(motor_zeroSpeed);
  STEER.writeMicroseconds(steering_middle);

  setJoystick();
}

// put your main code here, to run repeatedly:
void loop() {

  //take in analog input to read what values the joystick and RC controller are sending
  joy_motorvalue = analogRead(JOYVERT);
  joy_steervalue = analogRead(JOYHORI);


  //print out values to the serial monitor so that we can look at them
  Serial.print("Joy motor:          ");
  Serial.println(joy_motorvalue);
  Serial.print("Joy steer:          ");
  Serial.println(joy_steervalue);
  Serial.print("RC motor:           ");
  Serial.println(rc_motor.GetValue());
  Serial.print("RC steer:           ");
  Serial.println(rc_steer.GetValue());
  Serial.print("RC Emergency Stop:  ");
  Serial.println(rc_stop.GetValue());
  Serial.println();

  // TEST FOR EMERGENCY STOP
  if (rc_stop.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_stop.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE) {
    EmergencyStop();
  }

  // SEND SIGNAL TO MOTOR
  if (rc_motor.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_motor.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE) { // prioritize the rc controller
    MOTOR.writeMicroseconds(rc_motor.GetValue());
  }
  else {
    if (joy_motorvalue > JOY_MOTOR_MIDDLE + JOY_DEADZONE) {
      MOTOR.writeMicroseconds(motor_fwdSpeed);
    }
    else if (joy_motorvalue < JOY_MOTOR_MIDDLE - JOY_DEADZONE) {
      MOTOR.writeMicroseconds(motor_bwdSpeed);
    }
    else {
      MOTOR.writeMicroseconds(motor_zeroSpeed);
    }
  }
  
  // SEND SIGNAL TO STEERING
  if (rc_steer.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_steer.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE) { // prioritize the rc controller
    STEER.writeMicroseconds(rc_steer.GetValue());
  }
  else {
    if (joy_motorvalue > JOY_STEER_MIDDLE + JOY_DEADZONE) {
      STEER.writeMicroseconds(steering_right);
    }
    else if (joy_motorvalue < JOY_STEER_MIDDLE - JOY_DEADZONE) {
      STEER.writeMicroseconds(steering_left);
    }
    else {
      STEER.writeMicroseconds(steering_middle);
    }
  }
}

void EmergencyStop() {
  // does rc send stop consistently? or just once?
  // if consistently, wait for release of button
  //while (rc_stop);
}
