// encoder
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile long pulses = 0;

long lastencoderValue = 0;

#define ENCODER_A D7
#define ENCODER_B D5
#define ENCODER_SWITCH D0

#define STEPS_PER_ACTION 4 // encoder steps per knob click


void ICACHE_RAM_ATTR handleInterrupt() {
  int MSB = digitalRead(ENCODER_A); //MSB = most significant bit
  int LSB = digitalRead(ENCODER_B); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;

  lastEncoded = encoded; //store this value for next time
  pulses ++;

  uiState = (((encoderValue/STEPS_PER_ACTION) % UI_STATES) + UI_STATES) % UI_STATES;
}

void setupEncoder()
{
 
  pinMode(ENCODER_A, INPUT); 
  pinMode(ENCODER_B, INPUT);

  digitalWrite(ENCODER_A, LOW); //turn pullup resistor on
  digitalWrite(ENCODER_B, LOW); //turn pullup resistor on  

  attachInterrupt(ENCODER_A, handleInterrupt, CHANGE);
  attachInterrupt(ENCODER_B, handleInterrupt, CHANGE);
}

void loopEncoder()
{
  if( trace ){Serial.print("ui state ");
  Serial.print(uiState);
  Serial.print(" encoder count ");
  Serial.println(encoderValue);
  }
}

