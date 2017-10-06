#ifndef BELLOWS_H
#define BELLOWS_H

class Bellows
{
  public:
  Bellows( Servo _servo );
  void drive( float drive );

  float servoAngle;
  
  private:
  Servo servo;
};
#endif

