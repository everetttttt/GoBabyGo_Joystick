//GoBabyGo Wichita State University
//Spring 2024
//project by Everett Kernen and Jackson Truitt

//It should be noted this code uses a custom PCB to connect wires together. See details for that in the README, if I remember

#include <Servo.h> // enables PWM output to the drive and steer motors

//TODO:
//test (obviously)

const bool DEBUG = true;
const int DEBUG_FREQ = 10000;
int count = 0;

//PINS (ordered clockwise around the arduino nano every)
const int RC_SPEED = 9; //PWM
const int RC_STEERING = 8; //PWM
const int RC_EMERGENCYSTOP = 7; //PWM
const int STEERING_output = 5; //PWM - this controls either the linear actuator or servo, depending on which is selected to steer the car
const int MOTOR_output = 3; //PWM
const int JOYHORI = A6;
const int JOYVERT = A4;
const int ADJ_SPEED = A2;
const int ADJ_STEERINGRANGE = A1;
const int ADJ_STEERINGCENTER = A0;

//CONSTANT VALUES
int JOY_MOTOR_MIDDLE = 512;
int JOY_STEER_MIDDLE = 512;
const int JOY_DEADZONE = 130;         //how far away from center do you need to push the joystick before it's initialized?
int RC_SPEED_MIDDLE = 1500;
int RC_STEERING_MIDDLE = 1500;
int RC_STOP_MIDDLE = 1300;
const int RC_DEADZONE = 250;

const int motor_maxDistanceFromZero = 400; // distance from fwdSpeed and bwdSpeed, used for speed controller
int motor_bwdSpeed = 1100;
const int motor_zeroSpeed = 1500;
int motor_fwdSpeed = 1900;

const int steering_maxDistanceFromZero = 500;
int steering_left = 1300;
int steering_middle = 1500;
int steering_right = 1700;



//VARIABLES
int joy_motorValue;
int joy_steerValue;
int adj_speed;
int adj_steerCenter;
int adj_steerRange;

//Motor Controller (viewed by arduino as a servo), see https://www.arduino.cc/reference/en/libraries/servo/writemicroseconds/ for how we are controlling the "Servos"
Servo MOTOR;
Servo STEER;


// see https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/ for info on interrupts, Interrupt Service Routines (ISRs), etc.
class PWM {
  int pin;
  bool prevState;
  unsigned long time;
  int value = -1;

  public:
    PWM(int pwm_pin) {
      pin = pwm_pin;
    }
    void start() {
      prevState = digitalRead(pin);
      time = micros();
      pinMode(pin, INPUT);
    }

    int GetValue() { return value; }

    void InterruptServiceRoutine(int rc) {
      unsigned long currTime = micros();
      bool currState = digitalRead(pin);
      if (currState != prevState) {
        if (currState) { // switched from low to high
          time = currTime;
        } 
        else { // switched from high to low
          value = currTime - time;
          /*
          switch (rc) { // set rc middle values if not yet set
            case 0:
              RC_SPEED_MIDDLE = (RC_SPEED_MIDDLE == -1 ? value : RC_SPEED_MIDDLE);
              break;
            case 1:
              RC_STEERING_MIDDLE = (RC_STEERING_MIDDLE == -1 ? value : RC_STEERING_MIDDLE);
              break;
            case 2:
              RC_STOP_MIDDLE = (RC_STOP_MIDDLE == -1 ? value : RC_STOP_MIDDLE);
              break;
          }
          */
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
  pinMode(JOYHORI, INPUT);
  pinMode(JOYVERT, INPUT);
  pinMode(ADJ_SPEED, INPUT);
  pinMode(ADJ_STEERINGRANGE, INPUT);
  pinMode(ADJ_STEERINGCENTER, INPUT);
  
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

  setMiddles();
}

// put your main code here, to run repeatedly:
void loop() {

  //take in analog input to read what values the joystick is sending
  joy_motorValue = analogRead(JOYVERT);
  joy_steerValue = analogRead(JOYHORI);

  //read speed controller and set speed values
  //SetSpeed();
  SetSteer();


  //print out values to the serial monitor so that we can look at them
  if (DEBUG && count == DEBUG_FREQ) {
    Serial.print("Joy motor:          ");
    Serial.print(joy_motorValue);
    Serial.print("   Motor Middle:      ");
    Serial.println(JOY_MOTOR_MIDDLE);

    Serial.print("Joy steer:          ");
    Serial.print(joy_steerValue);
    Serial.print("   Steer Middle:      ");
    Serial.println(JOY_STEER_MIDDLE);

    Serial.print("RC motor:           ");
    Serial.print(rc_motor.GetValue());
    Serial.print("  RC Motor Middle:    ");
    Serial.println(RC_SPEED_MIDDLE);

    Serial.print("RC steer:           ");
    Serial.print(rc_steer.GetValue());
    Serial.print("  RC Steer Middle:    ");
    Serial.println(RC_STEERING_MIDDLE);

    Serial.print("RC Emergency Stop:  ");
    Serial.print(rc_stop.GetValue());
    Serial.print("  RC Stop Middle:    ");
    Serial.println(RC_STOP_MIDDLE);
    Serial.println();
    count = 0;
  }
  count++;

  // TEST FOR EMERGENCY STOP
  if (rc_stop.GetValue() > (RC_STOP_MIDDLE + RC_DEADZONE) || rc_stop.GetValue() < (RC_STOP_MIDDLE - RC_DEADZONE)) {
    Serial.println("EMERGENCY STOP ACTIVATED");
    EmergencyStop();
  }
  


  // SEND SIGNAL TO MOTOR
  if (rc_motor.GetValue() > (RC_SPEED_MIDDLE + RC_DEADZONE) || rc_motor.GetValue() < (RC_SPEED_MIDDLE - RC_DEADZONE)) { // prioritize the rc controller
    Serial.println("RC MOTOR OVERRIDE");
    int speed = map(rc_motor.GetValue(), 1000, 2000, motor_bwdSpeed, motor_fwdSpeed);
    MOTOR.writeMicroseconds(speed);
  }
  else if (joy_motorValue > (JOY_MOTOR_MIDDLE + JOY_DEADZONE) || joy_motorValue < (JOY_MOTOR_MIDDLE - JOY_DEADZONE)) {
    int speed = map(joy_motorValue, 0, 1023, motor_bwdSpeed, motor_fwdSpeed);
    MOTOR.writeMicroseconds(speed);
  }
  else {
    MOTOR.writeMicroseconds(motor_zeroSpeed);
  }


  // SEND SIGNAL TO STEERING
  if (rc_steer.GetValue() > (RC_STEERING_MIDDLE + RC_DEADZONE) || rc_steer.GetValue() < (RC_STEERING_MIDDLE - RC_DEADZONE)) { // prioritize the rc controller
    Serial.println("RC STEERING OVERRIDE");
    int steer = map(rc_steer.GetValue(), 1000, 2000, steering_left, steering_right);
    STEER.writeMicroseconds(steer);
  }
  else if (joy_steerValue > (JOY_STEER_MIDDLE + JOY_DEADZONE) || joy_steerValue < (JOY_STEER_MIDDLE - JOY_DEADZONE)) {
    int steer = map(joy_steerValue, 0, 1023, steering_left, steering_right);
    STEER.writeMicroseconds(steer);
  }
  else {
    STEER.writeMicroseconds(steering_middle);
  }

}

// ----------- HELPER FUNCTIONS ----------- //

void setMiddles() {
  JOY_MOTOR_MIDDLE = analogRead(JOYVERT);
  JOY_STEER_MIDDLE = analogRead(JOYHORI);
}

void SetSpeed() {
  adj_speed = analogRead(ADJ_SPEED);
  int distFromZero = map(adj_speed, 0, 1023, motor_maxDistanceFromZero / 3, motor_maxDistanceFromZero);
    // gives value in top two thirds of speed allowed
  motor_fwdSpeed = motor_zeroSpeed + distFromZero;
  motor_bwdSpeed = motor_zeroSpeed - distFromZero;
}

void SetSteer() {
  adj_steerCenter = analogRead(ADJ_STEERINGCENTER);
  adj_steerRange = analogRead(ADJ_STEERINGRANGE);
  steering_middle = map(adj_steerCenter, 0, 1023, 1400, 1600);
  int distFromCenter = map(adj_steerRange, 0, 1023, 50, steering_maxDistanceFromZero);
  steering_left = steering_middle - distFromCenter;
  steering_right = steering_middle + distFromCenter;
}

void EmergencyStop() {
  while (rc_stop.GetValue() < RC_STOP_MIDDLE - RC_DEADZONE ||
      rc_stop.GetValue() > RC_STOP_MIDDLE + RC_DEADZONE) {
        MOTOR.writeMicroseconds(motor_zeroSpeed);
        STEER.writeMicroseconds(steering_middle);
  }
}

void RCMotorInterrupt() { rc_motor.InterruptServiceRoutine(0); }
void RCSteerInterrupt() { rc_steer.InterruptServiceRoutine(1); }
void RCStopInterrupt() { rc_stop.InterruptServiceRoutine(2); }
