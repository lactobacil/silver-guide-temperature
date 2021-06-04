#include "EmonLib.h"             // Include Emon Library
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_MAX31865.h>

Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13);
EnergyMonitor emon1;             // Create an instance

#define RREF      430.0
#define RNOMINAL  100.0
#define VOLT_CAL 148.7
#define CURRENT_CAL 170.1


Adafruit_ADS1115 ads;
const float multiplier = 0.1875F;
float printval;


void setup()
{  
  Serial.begin(9600);
  
  emon1.voltage(A1, VOLT_CAL, 1.7);  // Voltage: input pin, calibration, phase_shift
  emon1.current(A0, CURRENT_CAL);       // Current: input pin, calibration.

  // ads.setGain(GAIN_TWOTHIRDS);  +/- 6.144V  1 bit = 0.1875mV (default)
  // ads.setGain(GAIN_ONE);        +/- 4.096V  1 bit = 0.125mV
  // ads.setGain(GAIN_TWO);        +/- 2.048V  1 bit = 0.0625mV
  // ads.setGain(GAIN_FOUR);       +/- 1.024V  1 bit = 0.03125mV
  // ads.setGain(GAIN_EIGHT);      +/- 0.512V  1 bit = 0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    +/- 0.256V  1 bit = 0.0078125mV
  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  ads.begin();
}

void loop()
{
  emon1.calcVI(20,2000);                                 // Calculate all. No.of half wavelengths (crossings), time-out
  float currentDraw     = emon1.Irms;                    //extract Irms into Variable
  float supplyVoltage   = emon1.Vrms;                    //extract Vrms into Variable
  //Serial.print("Current: ");
  Serial.println(currentDraw);

  int16_t adc0, adc1, adc2, adc3;
  adc0 = ads.readADC_SingleEnded(0);
  printval = (((adc0 * multiplier) - 2746)*152)/1440;    //Calculate ADC 0 

  uint16_t rtd = thermo.readRTD();
  Serial.print("RTD value: ");
  Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  //Serial.print("Ratio = "); Serial.println(ratio,8);
  //Serial.print("Resistance = "); Serial.println(RREF*ratio,8);
  Serial.print("Temperature = "); Serial.println(thermo.temperature(RNOMINAL, RREF));  //Temperature Measurement

  float temp = thermo.temperature(RNOMINAL, RREF);

// Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold"); 
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold"); 
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias"); 
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open"); 
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage"); 
    }
  }
  
  Serial.print("AIN0: ");
  Serial.println(printval); //calibrated ADC value from pressure
  delay(100);
}
