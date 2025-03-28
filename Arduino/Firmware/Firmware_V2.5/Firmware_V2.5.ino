//***********************************************************************
// Igual à versao 2.4 mas sem o Serial
//-----------------------------------------------------------------------

#include <Wire.h>
#include "RTClib.h"
#include "SdFat.h"
#include <SPI.h>
#include "HX711.h"
#include <avr/sleep.h>

//***********************************************************************
// Cartão SD
//-----------------------------------------------------------------------
const uint8_t chipSelect = SS;
SdFat sd;
SdFile file;
char fileName[16];
//***********************************************************************

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ERROS
//-----------------------------------------------------------------------
#define SDERROR   1
#define RTCERROR  2
#define FILEOVER  3
#define FILEERROR 4
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

//***********************************************************************
// LED de status
//-----------------------------------------------------------------------
#define WAKE 8 
#define ERRO 9
//***********************************************************************

//***********************************************************************
// Pino de interrupção (acroda príncipe)
//-----------------------------------------------------------------------
#define SQW 2
//***********************************************************************

volatile byte ShowTime = LOW;
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Inicializa RTC
//-----------------------------------------------------------------------
bool RTCInit(void){
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); // apenas a primeira vez para acertar RTC
    return(false);
  }
  return(true);
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Rotina de atendimento à interrupção de SQW
//-----------------------------------------------------------------------
void Acorda_Principe(){
  sleep_disable();    //Acorda principe
  detachInterrupt(0); //Inativa interrupções;
  digitalWrite(WAKE,HIGH);      // Principe acorda....
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Deep sleep
//-----------------------------------------------------------------------
void Dorme_Principe(){
    sleep_enable();                     // Habilita Sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);// Estabelece full sleep
    digitalWrite(WAKE,LOW);             // Principe vai dormir....
    attachInterrupt(0, Acorda_Principe, FALLING);
    sleep_cpu();                        // Principe a domir....
  }
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Rotina de datalogging
//-----------------------------------------------------------------------
void logData(void)
{
  char dataString[9];
  long Strain = (long)(scale.read_average(10)>>12);
  now = RTC.now();  // Amostra a hora/data
  snprintf(dataString, sizeof(dataString), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  // ************************************************************ Tenta abrir o ficheiro
  if (!file.open(fileName, O_WRITE | O_AT_END)) {
    erro(FILEERROR); // Erro durante datalog
  }
  // *********************************************************** Write Data
  file.print(dataString); // Mostra no terminal
  file.print("\t");
  file.println(Strain);
  //Força SD a atualizar para evitar perda de dados.
  if (!file.sync() || file.getWriteError()) {
    erro(FILEERROR); // Erro durante datalog
  }
  // *********************************************************** Fecha ficheiro
  file.close();
}
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Timestamp do ficheiro
//-----------------------------------------------------------------------
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = RTC.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// LED de ERRO
//-----------------------------------------------------------------------
void erro(uint8_t valor) {
  unsigned long ton = 0;
  unsigned long toff = 0;
  detachInterrupt(0); //Inativa interrupções
  switch(valor){
    case SDERROR:  ton  = 500;
                   toff = 1500;
                   break;
    case RTCERROR: ton  = 500;
                   toff = 500;
                   break;
    case FILEOVER: ton  = 1000;
                   toff = 1000;
                   break;
    case FILEERROR:ton = 1500;
                   toff = 500;
                   break;
    }
    while(1){
      digitalWrite(ERRO,HIGH);
      delay(ton);
      digitalWrite(ERRO,LOW);
      delay(toff);
    }
}


//***********************************************************************  
void setup () {
  //....................................................................................
  pinMode(WAKE,OUTPUT);                 // LED indica que o principe acordou
  pinMode(ERRO,OUTPUT);                 // LED indica que o principe acordou
  pinMode(SQW, INPUT_PULLUP);           // Pino 2 para acordar o principe  
  // ************************************************************ Inicializa HX711
  scale.begin(DT,SCK);
  // ************************************************************ Inicializa RTC
  if(!RTCInit()){          // Inicializa RTC
    erro(RTCERROR);        // Acende LED de erro....
  }
  RTC.writeSqwPinMode(SquareWave1HZ);
  // ************************************************************ Nome Parcial do FileName
  now = RTC.now();  // Amostra a hora/data
  String DateTimeString= "";
  DateTimeString = DateTimeString + String(now.year(),DEC) + "/" + String(now.month(),DEC) + "/" + String(now.day(),DEC);
  DateTimeString = DateTimeString + "   " + String(now.hour(),DEC) + ":" + String(now.minute(),DEC) + ":" + String(now.second(),DEC);
  // ************************************************************ Inicializa cartão de memória
  if (!sd.begin(chipSelect, SD_SCK_MHZ(50))) {
    erro(SDERROR); // Acende LED de erro...
  }
   // ************************************************************ Verifica o nome do ficheiro e se está já no cartão SD
  int n = 0;
  snprintf(fileName, sizeof(fileName), "%02d%02d%02d%02d.txt", now.year()-2000, now.month(), now.day(),n);
  while(sd.exists(fileName)) {
    n++;
    if(n>99){
      erro(FILEOVER);
    }
    snprintf(fileName, sizeof(fileName), "%02d%02d%02d%02d.txt", now.year()-2000, now.month(), now.day(),n);
    }
  // No máximo 99 ficheiros num dia....  se passar deve ser removidos ficheiros do cartão
  // ************************************************************ Tenta abrir o ficheiro
  SdFile::dateTimeCallback(dateTime);
  if (!file.open(fileName, O_WRONLY | O_CREAT | O_EXCL)) {
    erro(FILEERROR); // Wait forever since we cant write data
  }
  // ************************************************************ Escreve Cabeçalho
  file.println("(c)2019 The Donkey Paradise/IPB");
  file.print("File Created: ");
  file.println(DateTimeString);
  //************************************************************* Fecha Ficheiro
  file.close();
  //************************************************************* Configura INT0...

}

void loop () {
  Dorme_Principe();
  logData();

}
