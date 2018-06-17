#ifndef BELLOWS_H
#define BELLOWS_H

class Bellows
{
  public:
  Bellows( int servoNum );
  void driveServoAngle();
  void drive( float drive ); // set servo to this value
  void target( float targetFraction ); // move joints to this value
  void loop();

  int incrementVal(int index, int delta);
  int setManual(int index,boolean _manual);


  boolean manual;
  float targetFraction;
  float currentFraction;
  float servoAngle;
  float error;
  Node* nodes[2];
  
  private:
  //Servo servo;
  int servoNum;
};
#endif

