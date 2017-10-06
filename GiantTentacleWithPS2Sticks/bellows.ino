#include "bellows.h"

Bellows::Bellows( Servo _servo )
{
  this->servo = _servo;
}

void Bellows::drive( float drive ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  this->servoAngle = fmap( drive, 1.0, -1.0, 0.0, 180 );
  this->servo.write(this->servoAngle);
}

