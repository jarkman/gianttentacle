/**
// test node
Node nodes[]={
  Node(0,0,0,1,true) 
};

int numNodes = 1;

/ */

// real nodes
Node nodes[]={
  Node(0,0,0,1,true), 
  Node(1,210+490+300,2,3,true),
  Node(2,190+551+210,4,5,true),
  Node(3,80+480+190,6,7,true),
  Node(4,330+80,8,9,true)
};
int numNodes = 5;
/**/



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
  float targetX = 0;
  float targetY = 0;
  float targetHeading = 0;
  
  Node *previous = NULL;
  
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    //read sensors

    if( trace ) { Serial.print("node " ); Serial.print(i); Serial.println("callign node loop");}
    
    node->loop();

    if( trace ) Serial.println("back from node loop");
    
    // calculate x & y offsets
    if( previous != NULL )
    {

      if( node->heading < -350 || previous->heading < -350 )
        node->relativeAngle = 0; // if a sensor is bad, assume straight
      else
      {
        node->relativeAngle = node->heading - previous->heading;
        

        // normalise to -180 to 180
        while( node->relativeAngle < -180.0 )
          node->relativeAngle += 360.0;
  
        while( node->relativeAngle > 180.0 )
          node->relativeAngle -= 360.0;
      }
      node->normalisedHeading = previous->normalisedHeading + node->relativeAngle;
      
      if( trace ){Serial.print("relative angle "); Serial.println(node->relativeAngle);}
      
      x += node->length * sin(node->normalisedHeading * 3.14 / 180.0);
      y += node->length * cos(node->normalisedHeading * 3.14 / 180.0);

      node->x = x;
      node->y = y;

      targetHeading += node->targetAngle;
      
      targetX += node->length * sin( targetHeading * 3.14 / 180.0);
      targetY += node->length * cos( targetHeading * 3.14 / 180.0);

      node->targetX = targetX;
      node->targetY = targetY;
      
    }
    else
    {
      node->normalisedHeading = 0;
      node->relativeAngle = 0;
      node->x = x;
      node->y = y;
      node->targetX = x;
      node->targetY = y;
    }

  //Serial.println("in loopNodes");
  //  node->log();
    
    previous = node;
  }

  if( ! calibrateCompasses )
    if( traceNodes )
      logNodes();
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


char *threedigits( float f )
{
  snprintf(report, sizeof(report), "%3d", (int) f);
  return report;

}

char *twodigits( float f )
{
  snprintf(report, sizeof(report), "%2d", (int) f);
  return report;

}

void  printNodes(  )
{
  int fh = oled.getFontHeight();
  int y = 0;

  oled.setCursor(0,y);
  
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    
    //oled.print((int) 1 + node->index);
    
    oled.print("*");
    if( node->heading < -350 )
      oled.print( "---");
    else
      oled.print(threedigits(node->heading));
    
    if(node->leftRanger)
    {
      oled.print("<");
      if( node->leftRange < 0 )
        oled.print( "--");
      else
        oled.print(twodigits( node->leftRange/100.0));
      
    }
  
    if(node->rightRanger)
    {
      oled.print(">");
      if( node->rightRange < 0 )
        oled.print( "--");
      else
        oled.print(twodigits(node->rightRange/100.0));

    }
      y += fh; 
      oled.setCursor(0,y);
  
  }
}

void drawPose()
{
  float w = oled.getLCDWidth();
  float h = oled.getLCDHeight();

  float l = 0;
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    l += node->length;
    }

  float offset = h/3.0;

  float scale = (h-offset) / l;
  
  // show current position
  // start in bottom middle
  int xorigin = w/2;
  int yorigin = h-offset;
  
  int x0 = xorigin;
  int y0 = yorigin;

  Node *last = NULL;
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);

    if( last != NULL )
    {
      oled.line( x0 + last->x*scale, y0 - last->y*scale,  x0 + node->x*scale, y0 - node->y*scale); 
      oled.circle(x0 + node->x*scale, y0 - node->y*scale, 2);// stick a circle at the end to show which is the real line, as opposed to the target
    }
    last = node;
  }

  // and show target position
  x0 = xorigin;
  y0 = yorigin;

  last = NULL;
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);

    if( last != NULL )
    {
      oled.line( x0 + last->targetX*scale, y0 - last->targetY*scale,  x0 + node->targetX*scale, y0 - node->targetY*scale); 
    }
    last = node;
  }

  // show behaviour and direction
  int fh = oled.getFontHeight();
  int fw = oled.getFontWidth();
  int y = h - fh; // bottom line
  int x;

  char *s = behaviourLabel();
  int side = behaviourSide();
  
  if( side == -1 )
    x = 0;
  else
    x = w - strlen(s) * fw;
    
  oled.setCursor(x,y);
  oled.print(s);
}

