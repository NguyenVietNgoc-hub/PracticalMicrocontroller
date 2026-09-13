//Dùng timer1, tần số 50Hz, dùng prescaler 256, fast PWM

#include <avr/io.h>
#include <util/delay.h>

void Timer1_PWM_Init(void){
  DDRB |= (1<<PB1) | (1<<PB2); // 
  TCCR1A = (1<<COM1A1) | (1<<COM1B1) | (1<<WGM11); 
  TCCR1B = (1<<WGM12) | (1<<WGM13) | (1<<CS12); 
  ICR1 = 1249; 
}
void servo_angle(uint8_t angle){
  uint16_t pulse_width = 31 + ((uint32_t)angle * (156 - 31)) / 180;
  OCR1A = pulse_width; 
  OCR1B = pulse_width*8; 
}
int main(void){
  Timer1_PWM_Init();
  while(1){
    servo_angle(0);
    _delay_ms(500);

    servo_angle(90);
    _delay_ms(500);

    servo_angle(180);
    _delay_ms(500);

    servo_angle(90);
    _delay_ms(500);

    servo_angle(0);
    _delay_ms(500);
  }
}