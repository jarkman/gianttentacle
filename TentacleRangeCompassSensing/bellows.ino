#include "bellows.h"

#define MAX_BELLOWS_ANGLE 75.0 // in degrees
#define DRIVE_GAIN 2.0

Bellows::Bellows( Servo _servo )
{
  this->servo = _servo;
  error = -2;
  targetFraction = 0.0;
}

void Bellows::target( float _targetFraction ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  targetFraction = _targetFraction;
}

void Bellows::loop()
{
  float currentFraction = (nodes[0]->relativeAngle + nodes[1]->relativeAngle) /  (2.0 * MAX_BELLOWS_ANGLE);

  error = targetFraction - currentFraction;
  // simple linear feedback
  drive( error * DRIVE_GAIN);
  
}

void Bellows::drive( float drive ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  this->servoAngle = fmap( drive, 1.0, -1.0, 0.0, 180 );
  this->servo.write(this->servoAngle);
}

