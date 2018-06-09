#ifndef BELLOWS_H
#define BELLOWS_H

class Bellows
{
  public:
  Bellows( Servo _servo );
  void drive( float drive ); // set servo to this value
  void target( float targetFraction ); // move joints to this value
  void loop();

  float targetFraction;
  float servoAngle;
  float error;
  Node* nodes[2];
  
  private:
  Servo servo;
};
#endif

