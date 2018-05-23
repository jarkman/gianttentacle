// test node
Node nodes[]={
  Node(0,0,0,1,true),
  Node(1,500,1,2,true),
  Node(2,400,3,4,false),
  Node(3,300,5,5,true),
  Node(4,200,7,8,true)
};
/*
// real nodes
Node nodes[]={
  Node(0,0,0,1,false), 
  Node(1,500,1,2,true),
  Node(2,400,3,4,false),
  Node(3,300,5,5,true),
  Node(4,200,7,8,true)
};
*/

void setupNodes()
{
  for( Node node : nodes )
    node.setup();


  baseBellows.nodes[0] = &nodes[1];
  baseBellows.nodes[1] = &nodes[2];
  tipBellows.nodes[0] = &nodes[3];
  tipBellows.nodes[1] = &nodes[4];

}

void loopNodes()
{
  float x = 0;
  float y = 0;
  Node *previous = NULL;
  
  for( Node node : nodes )
  {
    //read sensors
    node.loop();

    // calculate x & y offsets
    if( previous != NULL )
    {
      node.relativeAngle = node.heading - previous->heading;

      // normalise to -180 to 180
      while( node.relativeAngle > -180.0 )
        node.relativeAngle -= 360.0;

      while( node.relativeAngle < 180.0 )
        node.relativeAngle += 360.0;
 
      x += node.length * sin(node.relativeAngle);
      y -= node.length * cos(node.relativeAngle);

      node.x = x;
      node.y = y;
    }

    node.log();
    
    previous = &node;
  }
  
  
}

void logNodes()
{
  for( Node node : nodes )
    node.log();
}

