#include "bellows.h"

#define MAX_BELLOWS_ANGLE 75.0 // in degrees
#define DRIVE_GAIN 2.0


#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)


Bellows::Bellows( int _servoNum )
{
  this->servoNum = _servoNum;
  error = -2;
  targetFraction = 0.0;
  nodes[0] = NULL;
  nodes[1] = NULL;
}

void Bellows::target( float _targetFraction ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  targetFraction = _targetFraction;
  
}

void Bellows::loop()
{
  /*
  drive(1.0);
  delay(500);
  drive(-1.0);
  delay(500);
  return;
  */
  if( nodes[0] == NULL || nodes[1]==NULL )
    return;
    
  float currentFraction = (nodes[0]->relativeAngle + nodes[1]->relativeAngle) /  (2.0 * MAX_BELLOWS_ANGLE);

  error = targetFraction - currentFraction;
  // simple linear feedback
  drive( error * DRIVE_GAIN);
  
}

void Bellows::drive( float drive ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  this->servoAngle = fmap( drive, 1.0, -1.0, 0.0, 180 );

  float pulseLen = fmap( servoAngle, 0, 180, SERVOMIN, SERVOMAX ); // map angle to pulse length in PWM count units
  
  pwm.setPWM(servoNum, 0, pulseLen);
  
  //this->servo.write(this->servoAngle);
}

