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
#define ACTION_FRUSTRATED 1
#define ACTION_CATCH 2
#define ACTION_EVADE 3
//#define ACTION_EXPLORE 4

int action = ACTION_NONE; // what are we doing right now ?

long lastTargetTime = 0;
long actionStartTime = 0;

int targetNode = -1;
int targetSide = 0; // 1 for right, -1 for left
int targetRange = 0;

#define FRUSTRATION_BASE_PERIOD 20.0
#define FRUSTRATION_TIP_PERIOD 10.0
#define FRUSTRATION_AMPLITUDE 0.4

// what position are we wiggling around ?
float frustrationCenterBase = 0.0;
float frustrationCenterTip = 0.0;

int behaviourSide()
{
  return targetSide;
}

char *behaviourLabel()
{
  if( action == ACTION_NONE )
    report[0]='\0';
  else if( action == ACTION_FRUSTRATED )
    snprintf(report, sizeof(report), "%c", actionInitial());
  else
     snprintf(report, sizeof(report), "%c%d", actionInitial(), targetNode );
   
  return report;  
}

boolean loopBehaviour()
 {
  if( ! enableBehaviour )
    return false;
    
  if( action != ACTION_NONE )
  {
    loopAction();
    return true;
  }

  float f = fabs(baseBellows.frustration) + fabs(tipBellows.frustration);
  if( f > 3 )
  {
    startFrustrated();
  }
  else
  {
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
  }
  mood -= loopSeconds;
  if( mood < 0 )
    mood = 0;

  return false;
 }

void loopAction()
{
  switch( action )
  {
    case ACTION_FRUSTRATED:
      loopFrustrated();
      break;
      
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

char actionInitial()
{
  switch( action )
  {
    case ACTION_FRUSTRATED:
      return 'f';
      break;
      
    case ACTION_EVADE:
      return 'e';
      break;
      
    case ACTION_CATCH:
      return 'c';
      break;
      
   default:
    return '-';
    break;
  }
}
void startFrustrated()
{
  if( traceBehaviour ) Serial.println("starting ACTION_FRUSTRATED");
  action = ACTION_FRUSTRATED;
  actionStartTime = millis();
  frustrationCenterBase = baseBellows.currentFraction;
  frustrationCenterTip = tipBellows.currentFraction; 
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

void loopFrustrated()
{
  if( ((baseBellows.frustration < FRUSTRATION_LIMIT && tipBellows.frustration < FRUSTRATION_LIMIT) && millis() - actionStartTime > 10000) ||
      millis() - actionStartTime > 20000 )
  {
    if( traceBehaviour ) Serial.println("stopping ACTION_FRUSTRATED");
  
    action = ACTION_NONE;
    return;
  }

  float basePhase = fmod((float)(millis() - actionStartTime) * 0.001 / FRUSTRATION_BASE_PERIOD, FRUSTRATION_BASE_PERIOD); // 0 to 1.0
  float tipPhase = fmod((float)(millis() - actionStartTime) * 0.001 / FRUSTRATION_TIP_PERIOD, FRUSTRATION_TIP_PERIOD); // 0 to 1.0


  baseBellows.target( frustrationCenterBase + sin( 2.0 * 3.14 * basePhase ) * FRUSTRATION_AMPLITUDE );
  tipBellows.target( frustrationCenterTip + sin( 2.0 * 3.14 * tipPhase ) * FRUSTRATION_AMPLITUDE );

  float restoreFraction = loopSeconds * 0.05;
  
  frustrationCenterBase *= 1.0-restoreFraction;
  frustrationCenterTip *= 1.0-restoreFraction;
/*
  // move the frustrated joint away from the source of frustration
  // if the other joint is unaffected, move that one in the other direction
  if( baseBellows.frustration < FRUSTRATION_LIMIT )
    baseBellows.incrementTarget(tipBellows.frustration * loopSeconds / 3.0);
  else
    baseBellows.incrementTarget(-baseBellows.frustration * loopSeconds / 3.0);

  if( tipBellows.frustration < FRUSTRATION_LIMIT )
    tipBellows.incrementTarget(baseBellows.frustration * loopSeconds / 3.0);
  else
    tipBellows.incrementTarget(-tipBellows.frustration * loopSeconds / 3.0);
    */
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

  mood -= loopSeconds;
  if( mood < 0 )
    mood = 0;
    
  bend( targetNode, loopSeconds * (float)(targetSide) / 2.0 ); // bend away
  
 }


void loopEvade()
{
  findTarget(); // leaves old target in place if it doesn't find one
  
  long duration = millis() - actionStartTime;
  if( millis() - lastTargetTime > 5000 ||  // no targets in 10s
      duration > 10000 )        // or just been doing this too long
  {
    if( traceBehaviour ) Serial.println("stopping ACTION_EVADE");
  
    action = ACTION_NONE;
    return;
  }

  mood += loopSeconds;
  if( mood > 100 )
    mood = 100;
    
  bend( targetNode, - loopSeconds * (float)(targetSide) / 2.0 ); // bend away
    
  
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
    // just bend the tip at first
    b1 = delta * fabs(tipBellows.targetFraction);
    b2 = delta;
  }

  baseBellows.incrementTarget(b1);
  tipBellows.incrementTarget(b2);
    
}

// find a new target, or leave the old one in place
 
boolean findTarget()
{
  
  int newTargetNode = -1;
  int newTargetSide = 0;
  float minRange = 10000;
 
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

  if( newTargetNode > -1 &&           // found a new target
    lastTargetTime - millis() > 3000) // don't swap too often
  {
    targetNode = newTargetNode;
    targetSide = newTargetSide;
    targetRange = minRange;
    lastTargetTime = millis();
  
    if( traceBehaviour )
    { 
      Serial.print( actionInitial());
      Serial.print(" new target node ");
      Serial.print(targetNode);
      Serial.print(" side ");
      Serial.print(targetSide);
      Serial.print(" range ");
      Serial.print(targetRange);
      Serial.println("");
    }
  
    return true;
  }

  return false;
}

