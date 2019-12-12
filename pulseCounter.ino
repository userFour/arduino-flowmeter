//------------------------------------------------------------------------------
// PULSE COUNTER NANO
//
//
// Author(s): MArkos Frazzer    Date: 2019-12-09
//
//------------------------------------------------------------------------------
//
// ANALOGUE PROCESSING FOR DIGITAL LOGIC
// SIGNAL --> 358n VOLTAGE FOLLOWER --> 358n AMPLIFIER (Gain 33)
// --> LOW PASS FILTER --> COMPARATOR (Schmitt Trigger) --> ARDUIIINO INTERRUPT
//
// Firmware for reading the legacy proteus flowmeters at TRIUMF labs
//
//------------------------------------------------------------------------------

#include <SPI.h>


#define INPUT_PIN 2
#define SS_PIN 8
#define RPIN 3
#define GPIN 5
#define BPIN 9

//------------------------------------------------------------------------------
// FXN HEADERS FXN HEADERS FXN HEADERS FXN HEADERS FXN HEADERS FXN HEADERS 
//------------------------------------------------------------------------------
void clearDisplaySPI();
void setBrightnessSPI(byte value);
void setDecimalsSPI(byte decimals);
void s7sSendStringSPI(String toSend);
int getSystemState();
void s7sDisplay(String toSend, int decimalPosition);
String pad(int padThis);
void blip();


//------------------------------------------------------------------------------
// CONSTS CONSTS CONSTS CONSTS CONSTS CONSTS CONSTS CONSTS CONSTS CONSTS 
//------------------------------------------------------------------------------
const byte S7S_ADDR = 0x71;
static const uint8_t SW_NAME[5] = {A1, A2, A3, A6, A7};
const String MODE[5] = {"1 AB", "1 BC", "1 DC", " 300", "300B"};
const float MAP_VAL[] = {9.41E-4, 2.01E-3, 6.64E-3, 1.28E-2, 4.57E-3};

//------------------------------------------------------------------------------
// GLOBAL VARIABLES GLOBAL VARIABLES GLOBAL VARIABLES GLOBAL VARIABLES 
//------------------------------------------------------------------------------

// Misc variables
bool inRange = true;
bool idle = true;
bool frequencyFlag = false;
long unsigned int previousMicros = 0;
int systemState = 0;
int averageIndex = 0;
int j = 0;
int intVal = 0;
int intFrac = 0;
long unsigned int lastTriggered = 0;
long unsigned int averagePulseDelay[10];
long unsigned int averageBuffer = 0;
float averageMicros = 0;
float period = 0.0;
float frequency = 0.0;
float flowRate = 0.0;

// Status LED variables
bool ledState = false;
int ledPin = RPIN;
int ledInterval = 0;  // Period of execution, changes depending on LEDState
int tOn = 1000;  // LED on time
int tOff = 50;  // LED off time
long unsigned int ledPreviousMillis = 0;
long unsigned int leddisplayMillis = 0;

// Display variables
int displayInterval = 1000;
int decimals = 2;
long unsigned int displayMillis = 0;


//------------------------------------------------------------------------------
// SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP
//------------------------------------------------------------------------------
void setup() {

  // Declare some local variables
  int i = 0;
  
  // Setup pinModes
  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);
  
  digitalWrite(RPIN, LOW);
  digitalWrite(GPIN, LOW);
  digitalWrite(BPIN, LOW);
  
  pinMode(INPUT_PIN, INPUT);

  for (i = 0; i <= 4; i++) {
    pinMode(SW_NAME[i], INPUT);
     
  }

  attachInterrupt(digitalPinToInterrupt(INPUT_PIN), increment, RISING);

  // Setup serial
  Serial.begin(115200);
  Serial.println();

  Serial.print(" //-------------------------------------------------- \n");
  Serial.print(" // PULSE COUNTER \n");
  Serial.print(" //-------------------------------------------------- \n\n");

  // -------- SPI initialization
  pinMode(SS_PIN, OUTPUT);  // Set the SS pin as an output
  digitalWrite(SS_PIN, HIGH);  // Set the SS pin HIGH
  SPI.begin();  // Begin SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
  // --------

  systemState = getSystemState();

  Serial.print(" System state: ");
  Serial.print(systemState);
  Serial.print(" \n");

  if (frequencyFlag) {
    s7sSendStringSPI("ECHO");
    delay(2000);
    
  } else {
    clearDisplaySPI();  // Clears display, resets cursor
    s7sSendStringSPI(MODE[systemState]);
    delay(2000);
    
  }
  
}

//------------------------------------------------------------------------------
// LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP
//------------------------------------------------------------------------------
void loop() {

  blip();

  // Display stuff
  if (millis() - displayMillis >= displayInterval) {

    displayMillis = millis();

    averageBuffer = 0;
    
    for(j = 0; j <= 9; j++) {
    averageBuffer += averagePulseDelay[j];
  
    }

    averageMicros = float(averageBuffer) / 10.0;

    period = averageMicros / 1000000.0;

    if (period > 0.0001) {
      frequency = 1.0 / period;

      if (frequency >= 20.0 && frequency <= 100.0) {
        inRange = true;
        
      } else {
        inRange = false;
        
      }

      if (frequencyFlag) {
        
        intVal = floor(frequency);
        intFrac = round(100 * (frequency - intVal));
        
        if (intVal >= 100) {
          decimals = 1;
          
        } else {
          decimals = 2;
          
        }
  
        s7sDisplay((pad(intVal) + pad(intFrac)), decimals);
        
        
      } else {
        flowRate = frequency * MAP_VAL[systemState];
        flowRate *= 60.0;
  
        intVal = floor(flowRate);
        intFrac = round(100 * (flowRate - intVal));

        if (intVal >= 100) {
          decimals = 1;
          
        } else {
          decimals = 2;
          
        }
  
        s7sDisplay(String(pad(intVal) + pad(intFrac)), decimals);
        
      }
      
    } else {
      s7sSendStringSPI("IDLE");
      
    }
    
  }

}

//------------------------------------------------------------------------------
void increment() {

  averagePulseDelay[averageIndex] = micros() - previousMicros;

  previousMicros = micros();

  lastTriggered = millis();

  if(averageIndex < 9) {
    averageIndex++;
      
  } else {
    averageIndex = 0;
    
  }

  return;

}

//------------------------------------------------------------------------------
void clearDisplaySPI() {
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x76);  // Clear display command
  digitalWrite(SS_PIN, HIGH);

  return;
  
}

//------------------------------------------------------------------------------
void setBrightnessSPI(byte value) {
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x7A);  // Set brightness command byte
  SPI.transfer(value);  // brightness data byte
  digitalWrite(SS_PIN, HIGH);

  return;
  
}

//------------------------------------------------------------------------------
void setDecimalsSPI(byte decimals) {
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x77);
  SPI.transfer(decimals);
  digitalWrite(SS_PIN, HIGH);

  return;
  
}

//------------------------------------------------------------------------------
void s7sSendStringSPI(String toSend) {
  
  digitalWrite(SS_PIN, LOW);
  
  for (int i = 0; i < 4; i++) {
    SPI.transfer(toSend[i]);
    
  }
  digitalWrite(SS_PIN, HIGH);

  return;
  
}

//------------------------------------------------------------------------------
int getSystemState() {

  // Declare some local variables
  int i = 0;
  int swStates[5] = {LOW, LOW, LOW, LOW, LOW};
  uint8_t integer = 0;
  

  // Read all the switch states
  for (i = 0; i <= 4; i++) {
    if (i < 3){
      swStates[i] = digitalRead(SW_NAME[i]);
      
    } else {
      // Pins A6, A7 are analogue input only => Use ternary operator
      // (a ? b : c) "if a then b otherwise c"
      swStates[i] = (analogRead(SW_NAME[i]) > 500) ? 1 : 0;
  
    }
  }

  // Take the bits out of the array, shift them into an integer to get state
  for (i = 2; i >= 0; i-- ) {
    integer = integer << 1;
    integer = integer + swStates[i];
    
  }

  if (swStates[4] == HIGH) {
    frequencyFlag = true;
    
  }

  return integer;

}

//------------------------------------------------------------------------------
void s7sDisplay(String toSend, int decimalPosition) {

    clearDisplaySPI();  // Clears display, resets cursor
    s7sSendStringSPI(toSend);

    if (decimalPosition == 1) {
      setDecimalsSPI(0b000100);  // (', :, .(4), .(3), .(2), .(1))
      
    } else if (decimalPosition == 2) {
      setDecimalsSPI(0b000010);
      
    }
    

    return;
  
}

//------------------------------------------------------------------------------
String pad(int padThis) {

  String returnString = String(padThis);

  if (padThis < 10) {
    returnString = "0" + returnString;
    
  }

  return returnString;
  
}

//------------------------------------------------------------------------------
void blip() { 

//  if (millis() - lastTriggered >= 5000) {
//    ledPin = RPIN;
//    
//  } else if (!inRange) {
//    ledPin = BPIN;
//    
//  } else {
//    ledPin = GPIN;
//    
//  }

  if ((millis() - ledPreviousMillis) >= ledInterval) {

    ledPreviousMillis = millis();
    
    if (ledState) {
      ledInterval = tOff;
      ledState = !ledState;
      
    } else {
      ledInterval = tOn;
      ledState = !ledState;
      
    }

    digitalWrite(ledPin, ledState);
  }
}

//------------------------------------------------------------------------------