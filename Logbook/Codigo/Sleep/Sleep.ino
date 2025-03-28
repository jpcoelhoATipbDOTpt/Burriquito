#include <avr/sleep.h>

#define acordaPrincipe 2 //Pin 2 acorda o principe

void setup() {
  pinMode(LED_BUILTIN,OUTPUT);          // LED indica que o principe acordou
  pinMode(acordaPrincipe,INPUT_PULLUP); // Pino 2 para acordar o principe
  attachInterrupt(0, Acorda_Principe, FALLING);  
  // _attachInterrupt(numero_interrupcao,funcao, modo)
  // numero_interrupcao = 0 => interrupcao externa pino 2
}

void loop() {
  digitalWrite(LED_BUILTIN,HIGH); //Principe acordado
  delay(5000);                    //Fica acordado 5 segundos
  Dorme_Principe();
}

void Dorme_Principe(){
    sleep_enable();                     // Habilita Sleep
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);// Estabelece full sleep
    digitalWrite(LED_BUILTIN,LOW);      // Principe vai dormir....
    sleep_cpu();                        // Principe a domir....
  }

void Acorda_Principe(){
  sleep_disable();   //Acorda principe
  detachInterrupt(0); //Inativa interrupções;
}
