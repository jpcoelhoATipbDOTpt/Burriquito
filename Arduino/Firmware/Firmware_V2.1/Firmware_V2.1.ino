#include <Wire.h>
#include "RTClib.h"
#include "SdFat.h"
#include <SPI.h>
#include "HX711.h"

//***********************************************************************
// Cartão SD
//-----------------------------------------------------------------------
const uint8_t chipSelect = SS;
SdFat sd;
SdFile file;
char fileName[16];
//***********************************************************************

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// HX711
//-----------------------------------------------------------------------
#define DT  5   // Data Out 
#define SCK 4   // Clock
HX711 scale;
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//***********************************************************************
// RTC
//-----------------------------------------------------------------------
RTC_DS1307 RTC;
DateTime now;
//***********************************************************************

volatile byte ShowTime = LOW;
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Inicializa RTC
//-----------------------------------------------------------------------
bool RTCInit(void){
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC failed"); // DEBUG
    //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    return(false);
  }
  Serial.println("RTC sucessfull"); // DEBUG
  return(true);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Rotina de atendimento à interrupção de SQW
//-----------------------------------------------------------------------
void isr() {
  ShowTime = HIGH;
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


//***********************************************************************  
void setup () {
  //....................................................................................
  // ************************************************************ Inicializa porto série (para Debug apenas):
  Serial.begin(9600);
  // ************************************************************ Inicializa HX711
  scale.begin(DT,SCK);
  // ************************************************************ Inicializa RTC
  if(!RTCInit()){          // Inicializa RTC
    // Acende LED de erro....
    while (1);
  }
  RTC.writeSqwPinMode(SquareWave1HZ);
  // ************************************************************ Nome Parcial do FileName
  now = RTC.now();  // Amostra a hora/data
  String DateTimeString= "";
  DateTimeString = DateTimeString + String(now.year(),DEC) + "/" + String(now.month(),DEC) + "/" + String(now.day(),DEC);
  DateTimeString = DateTimeString + "   " + String(now.hour(),DEC) + ":" + String(now.minute(),DEC) + ":" + String(now.second(),DEC);
  Serial.println(DateTimeString); // Mostra no terminal  
  // ************************************************************ Inicializa cartão de memória
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    // Acende LED de erro...
    Serial.println("Erro na inicializacao do cartao");
    while(1);
  }
  Serial.println("sucessfull");  // ... :)
   // ************************************************************ Verifica o nome do ficheiro e se está já no cartão SD
  int n = 0;
  snprintf(fileName, sizeof(fileName), "%02d%02d%02d%02d.txt", now.year()-2000, now.month(), now.day(),n);
  while(sd.exists(fileName)) {
    n++;
    if(n>99){
      // Não é possível criar mais ficheiros neste dia... acende LED
      Serial.println("Demasiados ficheiros neste dia");
      while(1);
    }
    snprintf(fileName, sizeof(fileName), "%02d%02d%02d%02d.txt", now.year()-2000, now.month(), now.day(),n);
    }
  // No máximo 99 ficheiros num dia....  se passar deve ser removidos ficheiros do cartão
  Serial.println(fileName);
  // ************************************************************ Tenta abrir o ficheiro
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1);
  }
  // ************************************************************ Escreve Cabeçalho
  file.println("(c)2019 The Donkey Paradise/IPB");
  Serial.println("(c)2019 The Donkey Paradise/IPB");
  file.print("File Created: ");
  Serial.print("File Created: ");
  file.println(DateTimeString);
  Serial.println(DateTimeString);
  //************************************************************ Fecha Ficheiro
  file.close();
  //************************************************************* Configura INT0...
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, isr, FALLING);
}

void loop () {
  if(ShowTime){
    detachInterrupt(0);
    //ShowTimeOnScreen();
    logData();
    ShowTime=LOW;
    attachInterrupt(0, isr, FALLING);
  }   
}

//void ShowTimeOnScreen(void){
//  char dataString[9];
//  DateTime now = RTC.now();
//  snprintf(dataString, sizeof(dataString), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
//  Serial.print(dataString); // Mostra no terminal
//  Serial.print("\t");
//  Serial.println(619);
//}

void logData(void)
{
  char dataString[9];
  long Strain = (long)(scale.read_average(10)>>12);
  now = RTC.now();  // Amostra a hora/data
  snprintf(dataString, sizeof(dataString), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  // ************************************************************ Tenta abrir o ficheiro
  if (!file.open(fileName, O_WRITE | O_AT_END)) {
    Serial.println("Deu erro aqui...");
    // Wait forever since we cant write data
    while (1);
  }
  // *********************************************************** Write Data
  file.print(dataString); // Mostra no terminal
  Serial.print(dataString);
  file.print("\t");
  Serial.print("\t");
  file.println(Strain);
  Serial.println(Strain);
  //Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) {
    Serial.print("Erro");
    while(1);
  }
  // *********************************************************** Fecha ficheiro
  file.close();
}
