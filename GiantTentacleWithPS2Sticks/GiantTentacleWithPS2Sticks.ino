
#include <Servo.h>

#include "bellows.h"

Servo baseServo; 
Servo tipServo; 

Bellows baseBellows( baseServo );
Bellows tipBellows( tipServo );

int controlPotPin = 0; 

float servoAngle;

float wavePeriod = 10000.0; // in millis
float waveAmplitude = 1.0; // 0 to 1.0
long waveStartT = 0;

// PS2 stick positions, in range -1.0 to +1.0
float stickLeftY = 0;
float stickLeftX = 0;
float stickRightY = 0;
float stickRightX = 0;

void setup() {

  Serial.begin(115200);
  
  PS2_setup();
  
  baseServo.attach(9); 
  tipServo.attach(8); 
  
  waveStartT = millis();
}

float wave(float phaseOffset) // -1.0 to 1.0
{
  float theta = 2 * PI * (millis() - waveStartT)/wavePeriod;

   
  return waveAmplitude * sin(theta + phaseOffset);
  
}

void loop() {
  
  float controlVal = analogRead(controlPotPin);   

  // waveFraction sets the amount of autowave we are going to add to the stick motion
  float waveFraction = fmap(controlVal, 0.0,905.0, 0.0, 1.0); // magic numbers for control pot
  float tipPhaseDelay = fmap(waveFraction, 0.0, 1.0, PI / 2.0, 0.0 ); // 90 degrees delay at small amplitude, for a wiggle, no delay at large amplitude, for a whole-tentacle curl

  wavePeriod = fmap( waveFraction, 0.0, 1.0, 10000.0, 20000.0 );
  
  baseBellows.drive(stickLeftX + waveFraction * wave(0));
  tipBellows.drive(stickRightX + waveFraction * wave(tipPhaseDelay));

   Serial.print(baseBellows.servoAngle);
   Serial.print("   ");
   Serial.println(tipBellows.servoAngle);
 
   PS2_loop();
   //PS2_loop_verbose();   
   delay( 50 );               
}



float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

