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
  targetAngle = 0;
  normalisedHeading = 0;
  
  leftRange = -1;
  rightRange = -1;
  compass = new LSM303();
  if( hasRange )
  {
    leftRanger = new VL53L0X();
    rightRanger = new VL53L0X();
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
  Serial.print("h ");
  Serial.print(heading);
  Serial.print("nh ");
  Serial.print(normalisedHeading);
  Serial.print("deg, ");
  if(leftRanger)
  {
    Serial.print("l ");
    Serial.print(leftRange/10.0);
    Serial.print("cm, ");
  }

  if(rightRanger)
  {
    Serial.print("r ");
    Serial.print(rightRange/10.0);
    Serial.print("cm, ");
  }
  Serial.println();
}

void Node::selectLeftMux()
{
  muxSelect( leftMuxAddress );
}

void Node::selectRightMux()
{
  muxSelect( rightMuxAddress );
}


void Node::setupRangers()
{
  if( leftRanger != NULL )
  {
    selectLeftMux();
    setupRanger(leftRanger);
    noMux();
  }

 if( rightRanger != NULL )
  {
    selectRightMux();
    setupRanger(rightRanger);
    noMux();
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

// we go with high speed because we have a lot of sensors to handle
//#define HIGH_SPEED
//#define HIGH_ACCURACY

{

  ranger->init();
  ranger->setTimeout(500); // if time is too long in single-measurement, we get general sluggishness and watchdog resets. Longer should be fine in continuous.

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

 // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  ranger->startContinuous();
  
}

void Node::setupCompass()
{
  selectLeftMux();
  compass->init();
  compass->enableDefault();
  compass->setTimeout(20); // ms
  
  /*
  Calibration values; the default values of +/-32767 for each axis
  lead to an assumed magnetometer bias of 0. Use the Calibrate example
  program to determine appropriate values for your particular unit.

  Or turn on calibrateCompasses!
  
  */

  // values from a single run of polulu calibrate:
  compass->m_min = (LSM303::vector<int16_t>){  -669,   -260,   -523};
  compass->m_max = (LSM303::vector<int16_t>){  +359,   +694,   +411};

  switch( index )
  {
  case 0: 
    compass->m_min = (LSM303::vector<int16_t>) {  -508,   -607,   -786};
    compass->m_max = (LSM303::vector<int16_t>) {  +543,   +479,   +172};
    break;
  
  case 1:
    compass->m_min = (LSM303::vector<int16_t>) {  -429,   -680,   -460};
    compass->m_max = (LSM303::vector<int16_t>) {  +652,   +500,   +644};
    break;
    
  case 2:
    compass->m_min = (LSM303::vector<int16_t>)  {  -589,   -715,   -637};
    compass->m_max = (LSM303::vector<int16_t>) {  +539,   +413,   +318};
    break;
    
  case 3:
    compass->m_min = (LSM303::vector<int16_t>)  {  -551,   -675,   -644};
    compass->m_max = (LSM303::vector<int16_t>) {  +451,   +476,   +315};
    break;
  
  case 4:
    compass->m_min = (LSM303::vector<int16_t>) {  -521,   -760,   -531};
    compass->m_max = (LSM303::vector<int16_t>)  {  +589,   +358,   +519};
    break;
  }
  
  noMux();
}

void Node::calibrateCompass() {  
  
  
    selectLeftMux();

  compass->read();
  
  running_min.x = min(running_min.x, compass->m.x);
  running_min.y = min(running_min.y, compass->m.y);
  running_min.z = min(running_min.z, compass->m.z);

  running_max.x = max(running_max.x, compass->m.x);
  running_max.y = max(running_max.y, compass->m.y);
  running_max.z = max(running_max.z, compass->m.z);

  
  snprintf(report, sizeof(report), "%d min: {%+6d, %+6d, %+6d}    max: {%+6d, %+6d, %+6d}",
    index, 
    running_min.x, running_min.y, running_min.z,
    running_max.x, running_max.y, running_max.z);
  Serial.println(report);

}


void Node::loop() {
  if( calibrateCompasses )
  {
    calibrateCompass();
    return;
  }
  
  if( trace ){Serial.println("---------------------");Serial.print("Node "); Serial.println(index);}
      
  if( trace ) Serial.println("...compass");
  long start = millis();
  loopCompass();
  if( trace ) Serial.println(millis() - start );   
  
  yield();  // console ESP8266 watchdog
  
  if( leftRanger != NULL )
  {
    start = millis();
    if( trace ) Serial.println("...left ranger");
    selectLeftMux();
    leftRange = readRanger(leftRanger);
    noMux();
    if( trace ) {Serial.print("...left : "); Serial.println(leftRange);}
    if( trace ) Serial.println(millis() - start ); 
    yield();  
  }

  if( rightRanger != NULL )
  {
    start = millis();
    if( trace ) Serial.println("...right ranger");
    selectRightMux();
    rightRange = readRanger(rightRanger);
    noMux();
    if( trace ) {Serial.print("...right : "); Serial.println(rightRange);}
    if( trace ) Serial.println(millis() - start ); 
    yield();  
  }
}

float Node::readRanger(VL53L0X* ranger)
{
  if( trace ) Serial.println("...reading ranger");
  float range = ranger->readRangeContinuousMillimeters();
  //float range = ranger->readRangeSingleMillimeters();
  if (ranger->timeoutOccurred()) 
  { 
    range = -1;
    if( trace ) Serial.println("...ranger TIMEOUT"); 
  }
  else
  {
    if( range > 500 )  // 500 determined by experiment, depends on the lighting //3000 )
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
  if( compass->timeoutOccurred() || heading < -360 || heading > 360 )
    heading = -360;


  if( trace ) Serial.print("heading: "); 
  if( trace ) Serial.println(heading);

  noMux();

}

