// This runs the giant inflated tentacle with 2 servo-controlled air valves,
// taking user input from a PS2 controller,
// with position feedback from LSM303 compass sensors (specifically, the Adafruit board)
// and range sensing with VL53L0X ToF range sensors (specifically, the CVJL53L0XV2 breakout from eBay)
// and a tca9548a i2c multiplexer on the Adafruit breakout

// We use Polulu libraries for the LSM303 and VL53L0X because they use a lot less memory than the Adafruit libraries

// Code lives in https://github.com/jarkman/gianttentacle

// It runs on a Nano


// Wiring:

// Nano A4 to SDA on the mux
// Nano A5 to SCL on the mux

// PS2 controller - 4 pins, 10,11,12,13

// Servos - 8,9 



// Each node has one compass sensor and either two or zero rangers

// i2c addresses:
// Mux    : 0x70
// LSM303 : 0x19 & 0x1E
// VL53L0X: 0x29 (but can be changed by reset-pin manipulation)


#include "Node.h"

#include <Servo.h>

#include "bellows.h"

boolean trace = false;          // activity tracing for finding crashes
boolean enableBellows = false;  // turn on/off bellows code

void setupNodes();
void loopNodes();

Servo baseServo; 
Servo tipServo; 

Bellows baseBellows( baseServo );
Bellows tipBellows( tipServo );

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

// pose targets for a boot-time wriggle selftest
#define NUM_SELFTEST 5
float selftest[NUM_SELFTEST][3] = {{0.0, 0.0}, {-1.0,-1.0}, {1.0,1.0}, {-1.0, 1.0},{0.0, 0.0}};
int nextSelftest = -1;
long selftestStartMillis = -1;

void setupI2C()
{
  Wire.begin();
  Wire.setClock(10000);  // 10k for a 10m wire length limit
}



void setup() {
  Serial.begin(9600);

  if( trace ) Serial.println("---Setup---");
  if( trace ) Serial.println("..i2c");
  setupI2C();

  if( trace ) Serial.println("..nodes");
  setupNodes();

  if( trace ) Serial.println("..ps2");
  //setupPS2();
  
  baseServo.attach(9); 
  tipServo.attach(8); 

  
  
  
  waveStartT = millis();
}

float wave(float phaseOffset) // -1.0 to 1.0
{
  float theta = 2 * PI * (millis() - waveStartT)/wavePeriod;

   
  return waveAmplitude * sin(theta + phaseOffset);
  
}

boolean loopSelftest()
{
  if( nextSelftest >= NUM_SELFTEST )
    return false; // selftest done

  float error = fabs( baseBellows.error ) + fabs(tipBellows.error);
  
  if( nextSelftest < 0 ||  // first time round the loop
    error < 0.1 ||          // arrived at target pose
    (selftestStartMillis > 0 && millis() - selftestStartMillis > 10000) ) // time has been too long, must be broken
  {
    //move on to next pose
    nextSelftest ++;
    selftestStartMillis = millis();
    
    if( nextSelftest >= NUM_SELFTEST )
      return false; // selftest done

    baseBellows.target(selftest[nextSelftest][0]);  
    tipBellows.target(selftest[nextSelftest][1]);  
  }
  return true;
}

void loopWave()
{
   
  //float controlVal = analogRead(controlPotPin);   

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
  
  baseBellows.target(stickLeftX +  effectiveWaveFraction * wave(0));
  tipBellows.target(tipDrive +  effectiveWaveFraction * wave(tipPhaseDelay));

}
void loop() {
  if( trace ) Serial.println("---Loop---");
  if( trace ) Serial.println("selftest/wave"); 
  if( ! loopSelftest())
    loopWave();

  if( trace ) Serial.println("ps2");
  //loopPS2();
  //PS2_loop_verbose();
  loopNodes();
  if( trace ) Serial.println("basebellows");
  baseBellows.loop();
  if( trace ) Serial.println("tipbellows");
  tipBellows.loop();
  if( trace ) Serial.println("done");

  logNodes();
  Serial.println("");
   delay(100);
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


