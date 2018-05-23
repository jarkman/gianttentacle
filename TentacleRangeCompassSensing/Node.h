#ifndef NODE_H

#define NODE_H

#include <Wire.h>
#include <LSM303.h> // polulu library
#include <VL53L0X.h> // polulu library - uses less memory than the adafruit one


class Node
{
  private:
    int index;
    
    LSM303* compass;  // from Polulu example
    VL53L0X* leftRanger;
    VL53L0X* rightRanger;
    int leftMuxAddress;
    int rightMuxAddress;
    boolean hasRange;
    
    

  public:
    float heading;
    float relativeAngle;
    float length;
    float leftRange;
    float rightRange; 
    // current position of the node end, where x is out-from-the-wall and y is left-right
    float x; 
    float y;

    Node( int _index, float _length, int _leftMux, int _rightMux, boolean _hasRange );

    void setup();
    void loop();
    void log();

  private:
    void Node::setupRangers();
    void Node::setupRanger(VL53L0X* ranger);
    void Node::setupCompass();
    float readRanger(VL53L0X* ranger);
    void loopCompass();
    void selectLeftMux();
    void selectRightMux();
    void Node::tcaselect(uint8_t i);
};

#endif NODE_H
