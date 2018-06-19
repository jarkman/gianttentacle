// This runs the giant inflated tentacle with 2 servo-controlled air valves,
// taking user input from a PS2 controller,
// with position feedback from LSM303 compass sensors (specifically, the Adafruit board)
// and range sensing with VL53L0X ToF range sensors (specifically, the CVJL53L0XV2 breakout from eBay)
// and a tca9548a i2c multiplexer on the Adafruit breakout

// We use Polulu libraries for the LSM303 and VL53L0X because they use a lot less memory than the Adafruit libraries

// Code lives in https://github.com/jarkman/gianttentacle

// It runs on a Wemos D1 with an oled screen


// Wiring:

// "Boot from Flash" requires some pin state to be right:

// D3/GPIO0 = 1 (high)
// D4/GPIO2 = 1 (high)
// D8/GPIO15 = 0 (low)

// so we'd better use these as outputs

// Pins are
//  I2C
//    D1 SCL
//    D2 SDA
//      wired to OLED and to mux and to servo board
// PS2 uses D3 D4 D6 D8
// Encoder is on D5 & D7, switch on D0


// Each node has one compass sensor and either two or zero rangers

// i2c addresses:
// Oled   : 0x3C
// Adafruit_PWMServoDriver: 0x40

// Mux    : 0x70, 0x71
// and beyound the mux:
//  LSM303 : 0x19 & 0x1E
//  VL53L0X: 0x29 (but can be changed by reset-pin manipulation)


#include "Node.h"
#include <Wire.h>
//#include <Servo.h>
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <Adafruit_PWMServoDriver.h>


#include "bellows.h"

boolean trace = true;          // activity tracing for finding crashes
boolean enableBellows = true;  // turn on/off bellows code
boolean enablePS2 = false;
boolean calibrateCompasses = false; // turn on then rotate each compass smoothly about all axes to get the individual compass min/max values for compass setup


void setupNodes();
void loopNodes();
void setupEncoder();



#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D

MicroOLED oled(PIN_RESET, DC_JUMPER);  // I2C Example

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

//Servo baseServo; 
//Servo tipServo; 
// servo numbers on the PWM servo driver
#define BASE_SERVO 0
#define TIP_SERVO 1

Bellows baseBellows( BASE_SERVO );
Bellows tipBellows( TIP_SERVO );


// UI screens accessible via encoder
#define UI_STATES 3
long uiState = 2;

char report[80];


float wavePeriod = 10000.0; // in millis
float waveFraction = 0.5; // 0 to 1.0

float waveAmplitude = 1.0; // 0 to 1.0
long waveStartT = 0;

// PS2 stick positions, in range -1.0 to +1.0
float stickLeftY = 0;
float stickLeftX = 0;
float stickRightY = 0;
float stickRightX = 0;

long lastStickMoveMillis = 0; // time when stick was last moved

// pose targets for a boot-time wriggle selftest
#define NUM_SELFTEST 6
float selftest[NUM_SELFTEST][2] = {{0.0, 0.0}, {-1.0,-1.0}, {-1.0, 1.0}, {1.0,-1.0}, {1.0, 1.0},{0.0, 0.0}};
int nextSelftest = -1;
long selftestStartMillis = -1;

void setupI2C()
{
  Wire.begin();
  Wire.setClock(10000);  // 10k for a 10m wire length limit
}



void setup() {
  setupDisplay();
  
  delay(5000);
  Serial.begin(115200);


  if( trace ) Serial.println("---Setup---");
  if( trace ) Serial.println("..i2c");
  setupI2C();

  if( trace ) Serial.println("..nodes");
  setupNodes();

  if( enablePS2 )
  {
    if( trace ) Serial.println("..ps2");
    setupPS2();
  }
  setupServoDriver();

  setupEncoder();
  
  //baseServo.attach(D7); 
  //tipServo.attach(D8); 

  waveStartT = millis();

  
}

void setupServoDriver()
{
  noMux();
  
  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
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
    Serial.print("Starting selftest pose ");
    Serial.println(nextSelftest);
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
  
  float tipPhaseDelay = - fmap( effectiveWaveFraction, 0.0, 1.0, - PI / 2.0, 0.3 ); // 90 degrees delay at small amplitude, for a wiggle, no delay at large amplitude, for a whole-tentacle curl

  //wavePeriod = fmap( waveFraction, 0.0, 1.0, 10000.0, 20000.0 );


  float tipDrive = stickLeftX * (1.5 * stickLeftY - 0.5) + stickRightX;
  
  baseBellows.target(stickLeftX +  effectiveWaveFraction * wave(0));
  tipBellows.target(tipDrive +  effectiveWaveFraction * wave(tipPhaseDelay));

}
void loop() {

  long start = millis();
  
  loopEncoder();
  loopDisplay();
 

  //delay(100);

  //return;
  
  if( trace ) Serial.println("---Loop---");
  if( trace ) Serial.println("selftest/wave"); 
  if( ! loopSelftest())
   loopWave();

  if(enablePS2)
  {
    if( trace ) Serial.println("ps2");
    //loopPS2();
    //PS2_loop_verbose();
  }
  loopNodes();
  if( trace ) Serial.println("basebellows");
  baseBellows.loop();
  if( trace ) Serial.println("tipbellows");
  tipBellows.loop();
  if( trace ) Serial.println("done");

  long end = millis();
  Serial.print("loop took ") ; Serial.println( end-start );
  
  //logNodes();
  Serial.println("");
   delay(100);
}



float fmapConstrained(float x, float in_min, float in_max, float out_min, float out_max)
{
  float f = fmap( x,  in_min, in_max, out_min, out_max);

  if( f < out_min )
    f = out_min;

  if( f > out_max )
    f = out_max;

  return f;
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


float fconstrain(float f, float out_min, float out_max)
{
  if( f < out_min )
    f = out_min;

  if( f > out_max )
    f = out_max;

  return f;
}

// https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/wiring-and-test
#define TCAADDR0 0x70
#define TCAADDR1 0x71
#define NO_MUX 16

// 16: no mux
// 0-7: mux0
// 8-15: mux1
 
void muxSelect(uint8_t i) {
  uint8_t m0;
  uint8_t m1;
  
  if( i > 15 )
  {
    // disable both
    m0 = 0;
    m1 = 0;  
  }
  else if( i < 8 )
  {
    // use mux 0, disable mux 1
    m0 = 1 << i;
    m1 = 0;
  }
  else if( i <= 15 )
  {
    // disable mux 0, use mux 1
    m0 = 0;
    m1 = 1 << (i-8);
  }

 
  Wire.beginTransmission(TCAADDR0);
  Wire.write(m0);
  Wire.endTransmission(); 
  
  Wire.beginTransmission(TCAADDR1);
  Wire.write(m1);
  Wire.endTransmission();  
}

// disable mux 
void noMux()
{
  muxSelect( NO_MUX );
}

