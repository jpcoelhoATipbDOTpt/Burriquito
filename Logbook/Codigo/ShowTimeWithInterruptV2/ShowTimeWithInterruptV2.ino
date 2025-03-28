#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

char DiasDaSemana[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

volatile byte ShowTime = LOW;

Ds1307SqwPinMode modes[] = {OFF, ON, SquareWave1HZ, SquareWave4kHz, SquareWave8kHz, SquareWave32kHz};

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
  
  rtc.writeSqwPinMode(modes[2]);
  
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
  String dataString = "";
  DateTime now = rtc.now();
  dataString = dataString + String(now.hour(),DEC) + ":" + String(now.minute(),DEC) + ":" + String(now.second(),DEC);
  Serial.print(dataString); // Mostra no terminal
  Serial.print("\t");
  Serial.println(619);
}
