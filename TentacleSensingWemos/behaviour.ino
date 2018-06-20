/* Behaviour
 *  
 *  Actions
 *   - Chasing/catching/releasing
 *   - Evading
 *   - Touching
 *   - Bored, exploring
 *   
 *  Moods 
 *   - Aggressive
 *   - Cautious
 *   - Friendly
 *   - Bored
 *  
 */


int mood = 50; // 0->100  bored->friendly->aggressive

// Actions will be perfromed till they are complete
#define ACTION_NONE 0
#define ACTION_CATCH 1
#define ACTION_EVADE 2
#define ACTION_EXPLORE 3

int action = ACTION_NONE; // what are we doing right now ?

long lastTargetTime = 0;
long actionStartTime = 0;

int targetNode = -1;
int targetSide = 0; // 1 for right, -1 for left

boolean loopBehaviour()
 {
  if( action != ACTION_NONE )
  {
    loopAction();
    return true;
  }

  if( findTarget())
  {
    // start a new action

    int r = random( 100 );
    if( r < mood )
      startCatch();  // in high mood, we catch
    else if( r < 100-mood )
      startEvade(); // in low mood, we evade

    return true;
  }

  mood -= 1;
  if( mood < 0 )
    mood = 0;

  return false;
 }

void loopAction()
{
  switch( action )
  {
    case ACTION_EVADE:
      loopEvade();
      break;
      
    case ACTION_CATCH:
      loopCatch();
      break;
   default:
    break;
  }
}

void startEvade()
{
  if( traceBehaviour ) Serial.println("starting ACTION_EVADE");
  action = ACTION_EVADE;
  actionStartTime = millis();
}

void startCatch()
{
  if( traceBehaviour ) Serial.println("starting ACTION_CATCH");
  
  action = ACTION_CATCH;
  actionStartTime = millis();
}

void loopCatch()
{
  findTarget(); // leaves old target in place if it doesn't find one
  
  long duration = millis() - actionStartTime;
  if( millis() - lastTargetTime > 10000 ||  // no targets in 10s
      duration > 30000 )        // or just been doing this too long
  {
    if( traceBehaviour ) Serial.println("stopping ACTION_CATCH");
  
    action = ACTION_NONE;
    return;
  }

  mood -= 1;
  if( mood < 0 )
    mood = 0;
    
  bend( targetNode,  targetSide / 100 ); // bend toward
 }


void loopEvade()
{
  findTarget(); // leaves old target in place if it doesn't find one
  
  long duration = millis() - actionStartTime;
  if( millis() - lastTargetTime > 10000 ||  // no targets in 10s
      duration > 30000 )        // or just been doing this too long
  {
    if( traceBehaviour ) Serial.println("stopping ACTION_EVADE");
  
    action = ACTION_NONE;
    return;
  }

  mood += 1;
  if( mood > 100 )
    mood = 100;
    
  bend( targetNode,  -targetSide / 100 ); // bend away
    
  
}

void bend( float node, float delta )
{
  float b1;
  float b2;
  
  if( node < 2 )
  {
    // bend both
    b1 = delta;
    b2 = delta;
  }
  else
  {
    // just bend the tip
    b1 = 0;
    b2 = delta;
  }

  baseBellows.incrementTargetFromPosition(b1);
  tipBellows.incrementTargetFromPosition(b2);
    
}

boolean findTarget()
{
  float minRange = 10000;
  
  int newTargetNode = -1;
  int newTargetSide = 0;
  
  for( int i = 0; i < numNodes; i ++ )
  {
    Node* node = &(nodes[i]);
    if( node->leftRange > 1 && node->leftRange < minRange)
    {
      minRange = node->leftRange;
      newTargetNode = i;
      newTargetSide = -1;
    }
    if( node->rightRange > 1 && node->rightRange < minRange)
    {
      minRange = node->rightRange;
      newTargetNode = i;
      newTargetSide = 1;
    }
  }

  if( newTargetNode > -1 )
  {
    targetNode = newTargetNode;
    targetSide = newTargetSide;
    lastTargetTime = millis();
  
    if( traceBehaviour )
    { 
      Serial.print("target node ");
      Serial.print(targetNode);
      Serial.print(" side ");
      Serial.print(targetSide);
    }
  
    return true;
  }

  return false;
}

