#include "Node.h"


Node::Node( int _index, float _length, int _leftMux, int _rightMux, boolean _hasRange )
{
  index = _index;
  length = _length;
  leftMuxAddress = _leftMux;
  rightMuxAddress = _rightMux;
  hasRange = _hasRange;
  heading = -360;
  relativeAngle = 0;
  leftRange = -1;
  rightRange = -1;
  compass = new LSM303();
  if( hasRange )
  {
    leftRanger = new VL53L0X();
    rightRanger = NULL; //new VL53L0X();
  }
  else
  {
    leftRanger = NULL;
    rightRanger = NULL;
  }
}

void Node::setup() {
  setupCompass();
  setupRangers();
}

void Node::log()
{
  Serial.print(heading);
  Serial.print(", ");
  if(leftRanger)
  {
    Serial.print(leftRange);
    Serial.print(", ");
  }

  if(rightRanger)
  {
    Serial.print(rightRange);
    Serial.print(", ");
  }
}
void Node::selectLeftMux()
{
  tcaselect( leftMuxAddress );
}

void Node::selectRightMux()
{
  tcaselect( rightMuxAddress );
}


// https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/wiring-and-test
#define TCAADDR 0x70
 
void Node::tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void Node::setupRangers()
{
  if( leftRanger != NULL )
  {
    selectLeftMux();
    setupRanger(leftRanger);
  }

 if( rightRanger != NULL )
  {
    selectRightMux();
    setupRanger(rightRanger);
  }
}

void Node::setupRanger(VL53L0X *ranger)
// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

#define LONG_RANGE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

#define HIGH_SPEED
//#define HIGH_ACCURACY

{

  ranger->init();
  ranger->setTimeout(500);

#if defined LONG_RANGE
  // lower the return signal rate limit (default is 0.25 MCPS)
  ranger->setSignalRateLimit(0.1);
  // increase laser pulse periods (defaults are 14 and 10 PCLKs)
  ranger->setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  ranger->setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  // reduce timing budget to 20 ms (default is about 33 ms)
  ranger->setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  // increase timing budget to 200 ms
  ranger->setMeasurementTimingBudget(200000);
#endif
}

void Node::setupCompass()
{
  selectLeftMux();
  compass->init();
  compass->enableDefault();
  
  /*
  Calibration values; the default values of +/-32767 for each axis
  lead to an assumed magnetometer bias of 0. Use the Calibrate example
  program to determine appropriate values for your particular unit.
  */

  // values from a single run of polulu calibrate:
  compass->m_min = (LSM303::vector<int16_t>){  -291,   -219,   -554};
  compass->m_max = (LSM303::vector<int16_t>){  -283,   -213,   -546};
}


void Node::loop() {
  if( trace ) Serial.println("...compass");
  loopCompass();
  
  if( leftRanger != NULL )
  {
    if( trace ) Serial.println("...left ranger");
    leftRange = readRanger(leftRanger);
    if( trace ) Serial.println("...left done");
  }

  if( rightRanger != NULL )
  {
    if( trace ) Serial.println("...right ranger");
    rightRange = readRanger(rightRanger);
    if( trace ) Serial.println("...right done");
  }
}

float Node::readRanger(VL53L0X* ranger)
{
  if( trace ) Serial.println("...reading ranger");
  float range = ranger->readRangeSingleMillimeters();
  if (ranger->timeoutOccurred()) 
  { 
    range = -1;
    if( trace ) Serial.println("...ranger TIMEOUT"); 
  }
  else
  {
    if( range > 3000 )
      range = -1;
      
    if( trace ) Serial.print("range: "); 
    if( trace ) Serial.println(range);
  }

  return range;
  
 
}
void Node::loopCompass() {

  if( trace ) Serial.println("....mux for compass");
  selectLeftMux();
  if( trace ) Serial.println("....reading compass");
  
  compass->read();
  if( trace ) Serial.println("....read compass");
  
  /*
  When given no arguments, the heading() function returns the angular
  difference in the horizontal plane between a default vector and
  north, in degrees.
  
  The default vector is chosen by the library to point along the
  surface of the PCB, in the direction of the top of the text on the
  silkscreen. This is the +X axis on the Pololu LSM303D carrier and
  the -Y axis on the Pololu LSM303DLHC, LSM303DLM, and LSM303DLH
  carriers.
  
  To use a different vector as a reference, use the version of heading()
  that takes a vector argument; for example, use
  
    compass.heading((LSM303::vector<int>){0, 0, 1});
  
  to use the +Z axis as a reference.
  */
  heading = compass->heading(); // in the range 0 to 360


  if( trace ) Serial.print("heading: "); 
  if( trace ) Serial.println(heading);

}

