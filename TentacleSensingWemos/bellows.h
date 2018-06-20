#ifndef BELLOWS_H
#define BELLOWS_H

class Bellows
{
  public:
  Bellows( int servoNum );
  void driveServoAngle();
  void drive( float drive ); // set servo to this value
  void target( float targetFraction ); // move joints to this value
  void incrementTarget( float delta );
  void incrementTargetFromPosition( float delta );
  void loop();

  int incrementManualVal(int index, int delta);
  int setManual(int index,boolean _manual);


  boolean manual;
  float targetFraction;
  float currentFraction;
  float servoAngle;
  float error; // -1 to 1
  float frustration; // integral of recent error, zeroed when we are on-target
  Node* nodes[2];
  
  private:
  //Servo servo;
  int servoNum;
};
#endif

