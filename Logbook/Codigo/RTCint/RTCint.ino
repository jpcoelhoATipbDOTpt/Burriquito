#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

int mode_index = 0;
volatile byte state = LOW;

Ds1307SqwPinMode modes[] = {OFF, ON, SquareWave1HZ, SquareWave4kHz, SquareWave8kHz, SquareWave32kHz};

void isr() {
  digitalWrite(13,state);
  state=!state;
}

void print_mode() {
  Ds1307SqwPinMode mode = rtc.readSqwPinMode();
  
  Serial.print("Sqw Pin Mode: ");
  switch(mode) {
  case OFF:             Serial.println("OFF");       break;
  case ON:              Serial.println("ON");        break;
  case SquareWave1HZ:   Serial.println("1Hz");       break;
  case SquareWave4kHz:  Serial.println("4.096kHz");  break;
  case SquareWave8kHz:  Serial.println("8.192kHz");  break;
  case SquareWave32kHz: Serial.println("32.768kHz"); break;
  default:              Serial.println("UNKNOWN");   break;
  }
}


void setup () {

  pinMode(13, OUTPUT);
  pinMode(2, INPUT_PULLUP);


  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  attachInterrupt(0, isr, FALLING);
}

void loop () {
  rtc.writeSqwPinMode(modes[mode_index++]);
  print_mode();

  if (mode_index > 5) {
    mode_index = 0;
  }

  delay(5000);
}