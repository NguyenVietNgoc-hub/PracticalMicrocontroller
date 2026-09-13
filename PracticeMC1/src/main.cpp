// #include <Arduino.h>
#include<avr/io.h>
#include<util/delay.h>
// const int led1 = 2;
// void setup(){
//   pinMode(led1, OUTPUT);
// }
// void loop(){
//   digitalWrite(led1, HIGH);
//   delay(500);
//   digitalWrite(led1, LOW);
//   delay(500);
// }

// const int led1 = 2;
// const int but1 = 3;
// void setup(){
//   pinMode(led1, OUTPUT);
//   pinMode(but1, INPUT_PULLUP);
// }
// void loop(){
//   if(digitalRead(but1) == LOW){
//     digitalWrite(led1, HIGH);
//   }else{
//     digitalWrite(led1, LOW);
//   }
// }

// const int buzz = 4;
// const int led1 = 2;
// const int but1 = 3;
// void setup(){
//   pinMode(led1, OUTPUT);
//   pinMode(buzz, OUTPUT);
//   pinMode(but1, INPUT_PULLUP);
// }
// void loop(){
//   if(digitalRead(but1) == LOW){
//     digitalWrite(led1, HIGH);
//     digitalWrite(buzz, HIGH);
//   }else{
//     digitalWrite(led1, LOW);
//     digitalWrite(buzz, LOW);
//   }
// }

// const int led1 = 2;
// const int but1 = 10;
// bool ledState = LOW;
// int lastButtonState = HIGH;
// void setup(){
//   pinMode(led1, OUTPUT);
//   pinMode(but1, INPUT_PULLUP);
//   digitalWrite(led1, ledState);
// }
// void loop(){
//   int buttonState = digitalRead(but1);
//   if(buttonState != lastButtonState){
//     if(buttonState == LOW){
//       ledState = !ledState;
//       digitalWrite(led1, ledState);
//     }
//     delay(50);
//   }
//   lastButtonState = buttonState;
// }

#define led1 PD2
#define but1 PB2
#define buzz PB4
// int main(void){
//   DDRD |= (1<<led1);
//   DDRB &= ~(1<<but1);
//   PORTB |= (1<<but1);
//   DDRB |= (1<<buzz);
//   while(1){
//     if(!(PINB & (1<<but1))){
//       PORTD |= (1<<led1);
//       PORTB |= (1<<buzz);
//     }else{
//       PORTD &= ~(1<<led1);
//       PORTB &= ~(1<<buzz);
//     }
//   }
// }
int main(void){
DDRD |= (1<<led1);
DDRB |= (1<<buzz);
DDRB &= ~(1<<but1);
PORTB |= (1<<but1);

PORTD &= ~(1<<led1);
PORTB &= ~(1<<buzz);

uint8_t laststate = 1;
uint8_t currentstate;
uint8_t ledstate = 0;
while(1){
  ledstate =!ledstate;
  if(!(PINB & (1<<but1))){
    currentstate = 0;
  } else {
    currentstate = 1;
  }
  if(laststate == 1 && currentstate == 0){
    if(ledstate == 1){
      PORTD |= (1<<led1);
      PORTB |= (1<<buzz);
    } else {
      PORTD &= ~(1<<led1);
      PORTB &= ~(1<<buzz);
    }
    _delay_ms(500);
  }
  laststate = currentstate;
}
}