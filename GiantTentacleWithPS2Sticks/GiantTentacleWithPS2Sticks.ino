
// This runs the giant inflated tentacle with 2 servo-controlled air valves,
// taking user input from a PS2 controller.
// Code lives in https://github.com/jarkman/gianttentacle

// It runs on a Nano

#include <Servo.h>

#include "bellows.h"

Servo baseServo; 
Servo tipServo; 

Bellows baseBellows( baseServo );
Bellows tipBellows( tipServo );

int controlPotPin = 0; 

float servoAngle;

float wavePeriod = 10000.0; // in millis
float waveFraction = 0.1; // 0 to 1.0

float waveAmplitude = 1.0; // 0 to 1.0
long waveStartT = 0;

// PS2 stick positions, in range -1.0 to +1.0
float stickLeftY = 0;
float stickLeftX = 0;
float stickRightY = 0;
float stickRightX = 0;

long lastStickMoveMillis = 0; // time when stick was last moved
 
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
  //float waveFraction = fmap(controlVal, 0.0,905.0, 0.0, 1.0); // magic numbers for control pot

  float effectiveWaveFraction = waveFraction;

  long stickDelay = millis() - lastStickMoveMillis; // how long since we last wiggled the stick ?
  if( stickDelay > 0 && stickDelay < 20000 )
    effectiveWaveFraction *= (float) (stickDelay + 2000) / (20000.0 + 2000.0); // fade back in over 20 secs, but always keep 10%

  //Serial.println(effectiveWaveFraction);
  
  float tipPhaseDelay = fmap( effectiveWaveFraction, 0.0, 1.0, - PI / 2.0, 0.0 ); // 90 degrees delay at small amplitude, for a wiggle, no delay at large amplitude, for a whole-tentacle curl

  //wavePeriod = fmap( waveFraction, 0.0, 1.0, 10000.0, 20000.0 );


  float tipDrive = stickLeftX * (1.5 * stickLeftY - 0.5) + stickRightX;
  
  baseBellows.drive(stickLeftX +  effectiveWaveFraction * wave(0));
  tipBellows.drive(tipDrive +  effectiveWaveFraction * wave(tipPhaseDelay));

  /*
   Serial.print(baseBellows.servoAngle);
   Serial.print("   ");
   Serial.println(tipBellows.servoAngle);
 */
   PS2_loop();
   //PS2_loop_verbose();   
   delay( 50 );               
}



float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

