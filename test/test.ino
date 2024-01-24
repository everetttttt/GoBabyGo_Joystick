/*==========================================================================
// Author : Handson Technology
// Project : BTD7960 Motor Control Board driven by Arduino.
// Description : Speed and direction controlled by a potentiometer attached
// to analog input A0. One side pin of the potentiometer (either one) to
// ground; the other side pin to +5V
// Source-Code : BTS7960.ino
// Program: Control DC motors using BTS7960 H Bridge Driver.
//==========================================================================
// Connection to the BTS7960 board:
// BTS7960 Pin 1 (RPWM) to Arduino pin 5(PWM)
// BTS7960 Pin 2 (LPWM) to Arduino pin 6(PWM)
// BTS7960 Pin 3 (R_EN), 4 (L_EN), 7 (VCC) to Arduino 5V pin
// BTS7960 Pin 8 (GND) to Arduino GND
// BTS7960 Pin 5 (R_IS) and 6 (L_IS) not connected
*/
const int RPWM_Output = 5; // Arduino PWM output pin 5; connect to IBT-2 pin 1 (RPWM)
const int LPWM_Output = 6; // Arduino PWM output pin 6; connect to IBT-2 pin 2 (LPWM)
const int yellow_in = 2;
const int white_in = 3;
const int blue_in = 4;

void setup() {
  pinMode(RPWM_Output, OUTPUT);
  pinMode(LPWM_Output, OUTPUT);
  
  pinMode(yellow_in, INPUT);
  pinMode(white_in, INPUT);
  pinMode(blue_in, INPUT);
}

void loop() {
  yellow = analogRead(yellow_in);
  white = analogRead(white_in);
  blue = analogRead(blue_in);
  Serial.print("yellow: ");
  Serial.print(yellow);
  Serial.print("   white: ");
  Serial.print(white);
  Serial.print("   blue: ");
  Serial.println(blue);

  if (millis() % 4096 > 2048) {
    analogWrite(LPWM_Output, 0);
    analogWrite(RPWM_Output, 255);
  }
  else {
    analogWrite(LPWM_Output, 255);
    analogWrite(RPWM_Output, 0);
  }
}