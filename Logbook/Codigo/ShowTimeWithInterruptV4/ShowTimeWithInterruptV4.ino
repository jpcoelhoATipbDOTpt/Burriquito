#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

volatile byte ShowTime = LOW;

void isr() {
  ShowTime = HIGH;
}

  
void setup () {

  Serial.begin(9600);
  if (! rtc.begin()) {
    // Vai acender o LED de Erro...
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    // Vai acender o LED de Erro...
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    while (1);
  }
  
  rtc.writeSqwPinMode(SquareWave1HZ);
  
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
