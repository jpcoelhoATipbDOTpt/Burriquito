#include <Wire.h>
#include "RTClib.h"
#include "SdFat.h"
#include <SPI.h>
#include "HX711.h"
#include "DebugUtils.h"

#define DEBUG // Comentar esta lista para sair do modo de DEBUG

//***********************************************************************
// Cartão SD
//-----------------------------------------------------------------------
// Chip select (pino 10).
const uint8_t chipSelect = SS;
// File system object.
SdFat sd;
// Log file.
SdFile file;
// Nome do ficheiro
char fileName[16];
//***********************************************************************

//***********************************************************************
// RTC
//-----------------------------------------------------------------------
RTC_DS1307 rtc;
//***********************************************************************


volatile byte ShowTime = LOW;

void isr() {
  ShowTime = HIGH;
}

//***********************************************************************  
void setup () {
  //.....................................................................
  // Porto Série 
  Serial.begin(9600); // Temporário... apenas para debug
  //.....................................................................
  // Configura RTC
  if (! rtc.begin()) {
    // Vai acender o LED de Erro...
    DEBUG_PRINTLN("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    // Vai acender o LED de Erro...
    DEBUG_PRINTLN("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    while (1);
  }
  
  rtc.writeSqwPinMode(SquareWave1HZ);
  //.....................................................................

  // Initialize at the highest speed supported by the board that is
  // not over 50 MHz. Try a lower speed if SPI errors occur.
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    sd.initErrorHalt();
  }
  
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, isr, FALLING);
}

void loop () {
  if(ShowTime){
    detachInterrupt(0);
    ShowTimeOnScreen();
    ShowTime=LOW;
    attachInterrupt(0, isr, FALLING);
  }   
}

void ShowTimeOnScreen(void){
  char dataString[9];
  DateTime now = rtc.now();
  snprintf(dataString, sizeof(dataString), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  Serial.print(dataString); // Mostra no terminal
  Serial.print("\t");
  Serial.println(619);
}
