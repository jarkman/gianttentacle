#include "bellows.h"

float servoLimit = 60; // constrain max servo angle to save air

#define MAX_BELLOWS_ANGLE 75.0 // in degrees
#define DRIVE_GAIN -2.0

// values tuned for the tentacle valve servos - 130/550
#define SERVOMIN  130 //150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  550 //600 // this is the 'maximum' pulse length count (out of 4096)


Bellows::Bellows( int _servoNum )
{
  this->servoNum = _servoNum;
  error = -2;
  targetFraction = 0.0;
  currentFraction = 0.0;
  frustration = 0;
  nodes[0] = NULL;
  nodes[1] = NULL;
  manual = false;
}

void Bellows::target( float _targetFraction ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  targetFraction = _targetFraction;
  
}

void Bellows::incrementTarget( float delta ) 
{
  targetFraction += delta;
  targetFraction = fconstrain( targetFraction, -1.0, 1.0 );
  
}

void Bellows::incrementTargetFromPosition( float delta ) 
{
  targetFraction = currentFraction + delta;
  targetFraction = fconstrain( targetFraction, -1.0, 1.0 );
  
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

  // nodes have a target angle for display purposes, give each one half the target
  nodes[0]->targetAngle = MAX_BELLOWS_ANGLE * targetFraction;
  nodes[1]->targetAngle = MAX_BELLOWS_ANGLE * targetFraction;
  
  error = targetFraction - currentFraction;
  if( trace ){Serial.print("nodes[0]->relativeAngle "); Serial.println(nodes[0]->relativeAngle);}
  if( trace ){Serial.print("nodes[1]->relativeAngle "); Serial.println(nodes[1]->relativeAngle);}
  if( trace ){Serial.print("targetFraction "); Serial.println(targetFraction);}
  if( trace ){Serial.print("currentFraction "); Serial.println(currentFraction);}
  if( trace ){Serial.print("error fraction "); Serial.println(error);}
  if( trace ){Serial.print("frustration "); Serial.println(frustration);}

  if( fabs(error) < FRUSTRATION_LIMIT )
    frustration = 0;
  else
    frustration += error * loopSeconds;
      
  // simple linear feedback
  drive( error * DRIVE_GAIN);
  
}

void Bellows::drive( float drive ) // 0 is off, 1.0 is full-left, -1.0 is full-right
{
  this->servoAngle = fmapConstrained( drive, 1.0, -1.0, 0.0, 180 );

  if( ! manual )
    driveServoAngle();
}


void Bellows::driveServoAngle()
{
  //servoAngle = 0;
  //servoAngle=180;
  
  if( trace ){Serial.print("servo angle "); Serial.println(this->servoAngle);}

  servoAngle = fconstrain( servoAngle, 90-servoLimit, 90+servoLimit);
  
  float pulseLen = fmap( servoAngle, 0, 180, SERVOMIN, SERVOMAX ); // map angle to pulse length in PWM count units

  noMux();
  
  pwm.setPWM(servoNum, 0, pulseLen);
  
  //this->servo.write(this->servoAngle);
}

int printOneBellows( int y, int fh, Bellows*b )
{
  oled.print(twodigits(b->currentFraction*100.0));

  oled.print(">");
  oled.print(twodigits(b->targetFraction*100.0));
  
  y += fh; 
  oled.setCursor(0,y); 

   
  oled.print("  S");
  oled.print(threedigits( b->servoAngle));
  
  oled.print(" f");
  oled.print(twodigits( b->frustration * 100.0));

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


int Bellows::incrementManualVal(int index, int delta)
{
  if( manual )
  {
    servoAngle += delta;
    servoAngle = fconstrain( servoAngle, 0, 180 );
   
    driveServoAngle();
  } 
}

int Bellows::setManual(int index,boolean _manual)
{
  manual = _manual;
}



