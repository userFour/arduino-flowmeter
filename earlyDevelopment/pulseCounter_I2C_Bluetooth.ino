//------------------------------------------------------------------------------
// PULSE COUNTER NANO V3
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
//------------------------------------------------------------------------------

#include <Wire.h>
#include <SoftwareSerial.h>
#include <HeartbeatLed.h>

#define inputPin 2


// Function Headers
void clearDisplayI2C();
void setBrightnessI2C(byte value);
void setDecimalsI2C(byte decimals);
void s7sSendStringI2C(String toSend);
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

long unsigned int tStart = 0;


// Some heartbeat settings
unsigned long sequence[] = {50, 1000};  // tOff, tOn


// Construct a heartbeat object
HeartbeatLed heartbeat(13, sequence, 2);

// Setup an instance of Software Serial
SoftwareSerial blueSerial(10, 11); // RX, TX 

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

  // Setup Software Serial
  blueSerial.begin(38400);  // Default baud rate

  // Setup Wire
  Wire.begin();  // Initialize hardware I2C pins

  clearDisplayI2C();  // Clears display, resets cursor
  s7sSendStringI2C("9999");
  
}

//------------------------------------------------------------------------------
// LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP
//------------------------------------------------------------------------------
void loop() {

  heartbeat.update();

  // Display stuff
  if (millis() - stateMillis >= stateInterval) {

    tStart = micros();

    stateMillis = millis();

    averageBuffer = 0;
    
    for(j = 0; j <= 9; j++) {
    averageBuffer += averagePulseDelay[i];
  
    }

    averageMicros = float(averageBuffer) / 10.0;

    blueSerial.print(averageMicros, 4);
    blueSerial.print("\t");
    
    period = averageMicros / 1000000.0;

    blueSerial.print(period, 4);
    blueSerial.print("\t");

    if (period > 0.0001) {
      frequency = 1.0 / period;
      
    } else {
      frequency = 9999.0000;
      
    }
    
    blueSerial.println(frequency, 4);

    flowRate = mapFloat(frequency, 20.0, 100.0, 1.9, 9.5);

    if (inRange) {
      flowRate = int(round(flowRate * 100.0));
      s7sDisplay(pad(flowRate));
      
    } else {
      clearDisplayI2C();  // Clears display, resets cursor
      s7sSendStringI2C("9999");
      
    }

    blueSerial.println(micros() - tStart);

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

//  blueSerial.println("Triggered");

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
void s7sDisplay(String toSend) {

    clearDisplayI2C();  // Clears display, resets cursor
    s7sSendStringI2C(toSend);
    setDecimalsI2C(0b000010);  // (', :, .(4), .(3), .(2), .(1))
  
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