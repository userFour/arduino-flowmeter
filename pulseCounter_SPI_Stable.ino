//------------------------------------------------------------------------------
// PULSE COUNTER NANO
//
//
// Author(s): MArkos Frazzer    Date: 2019-10-23
//
//------------------------------------------------------------------------------
//
// ANALOGUE PROCESSING FOR DIGITAL LOGIC
// SIGNAL --> 358n VOLTAGE FOLLOWER --> 358n AMPLIFIER (Gain 33)
// --> LOW PASS FILTER --> COMPARATOR (Schmitt Trigger) --> ARDUIIINO INTERRUPT
//
//
//
//------------------------------------------------------------------------------

#include <SPI.h>

#define inputPin 2
#define ssPin 8


// Function Headers
void clearDisplaySPI();
void setBrightnessSPI(byte value);
void setDecimalsSPI(byte decimals);
void s7sSendStringSPI(String toSend);
void s7sDisplay(String toSend);
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax);
String pad(int padThis);



// Declare some consts
const byte S7S_ADDR = 0x71;

// Declare some variables
bool inRange = true;
long unsigned int pulses = 0;
long unsigned int previousMicros = 0;
int i = 0;
int j = 0;
long unsigned int averagePulseDelay[10];
long unsigned int averageBuffer = 0;
float averageMicros = 0;
float period = 0.0;
float frequency = 0.0;
float flowRate = 0.0;

int stateInterval = 1000;
long unsigned int stateMillis = 0;


//------------------------------------------------------------------------------
// SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP
//------------------------------------------------------------------------------
void setup() {

  // Setup pinModes
  pinMode(inputPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(inputPin), increment, RISING);

//  // Setup serial
//  Serial.begin(115200);
//  Serial.println();
//
//  Serial.print(" //-------------------------------------------------- \n");
//  Serial.print(" // PULSE COUNTER \n");
//  Serial.print(" //-------------------------------------------------- \n\n");

  // -------- SPI initialization
  pinMode(ssPin, OUTPUT);  // Set the SS pin as an output
  digitalWrite(ssPin, HIGH);  // Set the SS pin HIGH
  SPI.begin();  // Begin SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
  // --------

  clearDisplaySPI();  // Clears display, resets cursor
  s7sSendStringSPI("9999");
  
}

//------------------------------------------------------------------------------
// LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP
//------------------------------------------------------------------------------
void loop() {

  // Display stuff
  if (millis() - stateMillis >= stateInterval) {

    stateMillis = millis();

    averageBuffer = 0;
    
    for(j = 0; j <= 9; j++) {
    averageBuffer += averagePulseDelay[i];
  
    }

    averageMicros = float(averageBuffer) / 10.0;

    period = averageMicros / 1000000.0;

    if (period > 0.0001) {
      frequency = 1.0 / period;
      
    } else {
      frequency = 9999.0000;
      
    }

    s7sDisplay(String(round(frequency * 100.0)));
    
//    flowRate = mapFloat(frequency, 20.0, 100.0, 1.9, 9.5);
//
//    if (inRange) {
//      flowRate = int(round(frequency));
//      s7sSendStringSPI(pad(flowRate));
//      
//    } else {
//      clearDisplaySPI();  // Clears display, resets cursor
//      s7sSendStringSPI("9999");
//      
//    }

  }

}

//------------------------------------------------------------------------------
void increment() {

  pulses++;

  averagePulseDelay[i] = micros() - previousMicros;

  previousMicros = micros();

  if(i < 9) {
    i++;
      
  } else {
    i = 0;
    
  }

}

//------------------------------------------------------------------------------
void clearDisplaySPI() {
  
  digitalWrite(ssPin, LOW);
  SPI.transfer(0x76);  // Clear display command
  digitalWrite(ssPin, HIGH);
  
}

//------------------------------------------------------------------------------
void setBrightnessSPI(byte value) {
  
  digitalWrite(ssPin, LOW);
  SPI.transfer(0x7A);  // Set brightness command byte
  SPI.transfer(value);  // brightness data byte
  digitalWrite(ssPin, HIGH);
  
}

//------------------------------------------------------------------------------
void setDecimalsSPI(byte decimals) {
  
  digitalWrite(ssPin, LOW);
  SPI.transfer(0x77);
  SPI.transfer(decimals);
  digitalWrite(ssPin, HIGH);
  
}

//------------------------------------------------------------------------------
void s7sSendStringSPI(String toSend) {
  
  digitalWrite(ssPin, LOW);
  
  for (int i=0; i<4; i++) {
    SPI.transfer(toSend[i]);
    
  }
  digitalWrite(ssPin, HIGH);
  
}

//------------------------------------------------------------------------------
void s7sDisplay(String toSend) {

    clearDisplaySPI();  // Clears display, resets cursor
    s7sSendStringSPI(toSend);
    setDecimalsSPI(0b000010);  // (', :, .(4), .(3), .(2), .(1))
  
}

//------------------------------------------------------------------------------
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {

  float returnVal = 0.0;

  if(x >= inMin && x <= inMax){
    inRange = true;
    returnVal = (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
    
  } else {
    inRange = false;
    returnVal = 0.0;
       
  }
  
  return returnVal;
 
}

//------------------------------------------------------------------------------
String pad(int padThis) {

  String returnString = String(padThis);

  if (padThis < 10) {
    returnString = "000" + returnString;
    
  }

  if (padThis < 100 && padThis >= 10) {
    returnString = "00" + returnString;
    
  }
  
  if (padThis < 1000 && padThis >= 100) {
    returnString = "0" + returnString;
  
  }

  return returnString;
  
}

//------------------------------------------------------------------------------
