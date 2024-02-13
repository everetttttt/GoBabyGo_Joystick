#include <Servo.h>

int delay_ms = 100;

int POTENTIOMETER = A6; // center pin of the potentiometer
int PWM_Output = 10; //pwm pin to motor controller
Servo motorController;


void setup()
{
  Serial.begin(9600);

  pinMode(POTENTIOMETER, INPUT);
  //pinMode(PWM_Output, OUTPUT);
  motorController.attach(PWM_Output);
  motorController.writeMicroseconds(1500);
}
void loop()
{
  int sensorValue = analogRead(POTENTIOMETER);
    sensorValue = map(sensorValue, 0, 1023, 1000, 2000);
  Serial.print("Pot: ");
  Serial.println(sensorValue);

  //analogWrite(PWM_Output, sensorValue);
  motorController.write(sensorValue);
  /*
  // sensor value is in the range 0 to 1023
  // the lower half of it we use for reverse rotation; the upper half for forward rotation
  if (sensorValue < 512)
  {
  // reverse rotation
    int reversePWM = -(sensorValue - 511) / 2;
    analogWrite(LPWM_Output, 0);
    analogWrite(RPWM_Output, reversePWM);
  }
  else
  {
    // forward rotation
    int forwardPWM = (sensorValue - 512) / 2;
    analogWrite(LPWM_Output, forwardPWM);
    analogWrite(RPWM_Output, 0);
  }
  */
  delay(delay_ms);
}
