//------------------------------------------------------------------------------
// PULSE COUNTER
//
//
// Author(s): MArkos Frazzer    Date: 2019-10-23
//
//------------------------------------------------------------------------------
//
// ANALOGUE COMPUTING FOR DIGITAL LOGIC
//
//------------------------------------------------------------------------------

#include <Wire.h>

#define led PC13
#define inputPin PA0


// Function Headers
void clearDisplayI2C();
void setBrightnessI2C(byte value);
void setDecimalsI2C(byte decimals);
void s7sSendStringI2C(String toSend);
void s7sDisplay(String toSend);
float mapFloat(float x, float inMin, float inMax, float outMin, float outMax);
String pad(int padThis);
void Blip();


// Declare some consts
const byte S7S_ADDR = 0x71;

// Declare some variables
bool inRange = true;
long unsigned int pulses = 0;
long unsigned int lastPulses = 0;
long unsigned int previousMillis = 0;
int i = 0;
int averagePulseDelay[10];
int averageMillis = 0;
float period = 0.0;
float frequency = 0.0;
int dummy = 0;
float flowRate = 0.0;

long unsigned int ledPreviousMillis = 0;
int interval = 0;  // Period of execution, changes depending on ledState
int tOn = 1000;
int tOff = 50;
bool ledState = true;



//------------------------------------------------------------------------------
// SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP SET UP
//------------------------------------------------------------------------------
void setup() {

  // Setup pinModes
  pinMode(led, OUTPUT);
  interval = tOn;
  
  pinMode(inputPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(inputPin), increment, FALLING);

  // Setup serial
  Serial.begin(115200);
  Serial.println();

  Serial.print(" //-------------------------------------------------- \n");
  Serial.print(" // PULSE COUNTER \n");
  Serial.print(" //-------------------------------------------------- \n\n");

  // Setup Wire
  Wire.begin();  // (SDA, SCL)

  clearDisplayI2C();  // Clears display, resets cursor
  s7sSendStringI2C("9999");
  
}

//------------------------------------------------------------------------------
// LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP
//------------------------------------------------------------------------------
void loop() {

  Blip();

  if (pulses > lastPulses) {

    lastPulses = pulses;  

    for(i = 8; i >= 0; i--){
      averagePulseDelay[i + 1] = averagePulseDelay[i];
      
    }

    averagePulseDelay[0] = millis() - previousMillis;

    for(i = 0; i <= 9; i++) {
      averageMillis += averagePulseDelay[i];
      
    }

    averageMillis /= 10;
    period = float(averageMillis) / 1000.0;
    
    frequency = 1.0 / period;

    dummy = int(round(frequency));

    clearDisplayI2C();  // Clears display, resets cursor
    s7sSendStringI2C(pad(dummy));
//
//    flowRate = mapFloat(frequency, 20.0, 100.0, 1.9, 9.5);
//
//    if (inRange) {
//      flowRate = int(round(flowRate * 100.0));
//      s7sDisplay(pad(flowRate));
//      
//    } else {
//      clearDisplayI2C();  // Clears display, resets cursor
//      s7sSendStringI2C("9999");
//      
//    }

    previousMillis = millis();
   
  }

}


//------------------------------------------------------------------------------
void increment() {

  pulses++;


}

//------------------------------------------------------------------------------
void clearDisplayI2C() {
  Wire.beginTransmission(S7S_ADDR);
  Wire.write(0x76);  // Clear display command
  Wire.endTransmission();
  
}

//------------------------------------------------------------------------------
void setBrightnessI2C(byte value) {
  Wire.beginTransmission(S7S_ADDR);
  Wire.write(0x7A);  // Set brightness command byte
  Wire.write(value);  // brightness data byte
  Wire.endTransmission();
  
}

//------------------------------------------------------------------------------
void setDecimalsI2C(byte decimals) {

//  Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)

  Wire.beginTransmission(S7S_ADDR);
  Wire.write(0x77);
  Wire.write(decimals);
  Wire.endTransmission();
  
}
//------------------------------------------------------------------------------
void s7sSendStringI2C(String toSend) {

  int counter = 0;

  Wire.beginTransmission(S7S_ADDR);
  
  for (counter = 0; counter < 4; counter++) {
    Wire.write(toSend[counter]);
    
  }
  
  Wire.endTransmission();
  
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
void s7sDisplay(String toSend) {

    clearDisplayI2C();  // Clears display, resets cursor
    s7sSendStringI2C(toSend);
    setDecimalsI2C(0b000010);  // (', :, .(4), .(3), .(2), .(1))
  
}

//------------------------------------------------------------------------------
void Blip() {

  if ((millis() - ledPreviousMillis) >= interval) {

    ledPreviousMillis = millis();
    
    if (ledState) {
      interval = tOn;
      ledState = !ledState;
      
    } else {
      interval = tOff;
      ledState = !ledState;
      
    }

    digitalWrite(led, ledState);
  }
}

//------------------------------------------------------------------------------