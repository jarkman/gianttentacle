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
  currentFraction = 0.0;
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
    
  currentFraction = (nodes[0]->relativeAngle + nodes[1]->relativeAngle) /  (2.0 * MAX_BELLOWS_ANGLE);

  error = targetFraction - currentFraction;
  // simple linear feedback
  drive( error * DRIVE_GAIN);
  
}

void Bellows::drive( float drive ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  this->servoAngle = fmapConstrained( drive, 1.0, -1.0, 0.0, 180 );

  float pulseLen = fmap( servoAngle, 0, 180, SERVOMIN, SERVOMAX ); // map angle to pulse length in PWM count units

  noMux();
  
  pwm.setPWM(servoNum, 0, pulseLen);
  
  //this->servo.write(this->servoAngle);
}

int printOneBellows( int y, int fh, Bellows*b )
{
  oled.print("T:");
  oled.print((int) (b->targetFraction*100.0));
  
  oled.print(" C:");
  oled.print((int) (b->currentFraction*100.0));

  y += fh; 
  oled.setCursor(0,y); 

   
  oled.print("  S ");
  oled.print((int) b->servoAngle);

  y += fh; 
  oled.setCursor(0,y); 
  
  return y;
}

void  printBellows(  )
{
  int fh = oled.getFontHeight();
  int y = 0;

  oled.setCursor(0,y);

  oled.print("Bellows:");
  
  y += fh; 
  oled.setCursor(0,y); 
  
  if( enableBellows )
  {
    y = printOneBellows( y, fh, &baseBellows );
    y = printOneBellows( y, fh, &tipBellows );
  } 
 
}


