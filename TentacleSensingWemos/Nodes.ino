/*
// test node
Node nodes[]={
  Node(0,0,0,1,true) 
};

int numNodes = 1;

*/

// real nodes
Node nodes[]={
  Node(0,0,0,0,false), 
  Node(1,500,1,2,true),
  Node(2,400,3,4,true),
  Node(3,300,5,5,true),
  Node(4,200,7,8,true)
};
int numNodes = 5;
/**/

void  printNodes(  )
{
  int fh = oled.getFontHeight();
  int y = 0;

  oled.setCursor(0,y);
  
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    
    oled.print((int) node->index);
    
    oled.print("*");
    oled.print((int) node->heading);
    
    if(node->leftRanger)
    {
      oled.print("<");
      oled.print((int) (node->leftRange/10.0));
      
    }
  
    if(node->rightRanger)
    {
      oled.print(">");
      oled.print((int) (node->rightRange/10.0));

    }
      y += fh; 
      oled.setCursor(0,y);
  
  }
}


void setupNodes()
{
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    node->setup();
  }
  
  if( enableBellows )
  {
  baseBellows.nodes[0] = &nodes[1];
  baseBellows.nodes[1] = &nodes[2];
  tipBellows.nodes[0] = &nodes[3];
  tipBellows.nodes[1] = &nodes[4];
  }

}

void loopNodes()
{
  float x = 0;
  float y = 0;
  Node *previous = NULL;
  
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    //read sensors
    node->loop();

    // calculate x & y offsets
    if( previous != NULL )
    {
      node->relativeAngle = node->heading - previous->heading;

      // normalise to -180 to 180
      while( node->relativeAngle > -180.0 )
        node->relativeAngle -= 360.0;

      while( node->relativeAngle < 180.0 )
        node->relativeAngle += 360.0;
 
      x += node->length * sin(node->relativeAngle);
      y -= node->length * cos(node->relativeAngle);

      node->x = x;
      node->y = y;
    }

  //Serial.println("in loopNodes");
  //  node->log();
    
    previous = node;
  }

/*
  Serial.println("after loopNodes");
    for( Node node : nodes )
  { node.log(); }
  Serial.println("--");
*/
}

void logNodes()
{
   Serial.println("");
    Serial.println("Nodes");
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    Serial.println("");
    Serial.print("Node ");
    Serial.println(node->index);
    node->log();
  }
   Serial.println("  ----");
}

