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

// target location of action
float actionBellows = 0;
float actionBellows1;

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
  action = ACTION_EVADE;
}

void startCatch()
{
  action = ACTION_CATCH;
}

void loopCatch()
{//TODO
 }

int evadeNode = -1;
int targetSide = 0; // 1 for right, -1 for left

void loopEvade()
{
  findTarget(); // leaves old target in place if it doesn't find one
  

  if( millis() - lastTargetTime > 10000 ||  // no targets in 10s
      random( 300 ) == 0 )        // or just been doing this too long
  {
    action = ACTION_NONE;
    return;
  }

  mood += 1;
  if( mood > 100 )
    mood = 100;
    
  move towards target
    
  
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
    return true;
  }

  return false;
}

