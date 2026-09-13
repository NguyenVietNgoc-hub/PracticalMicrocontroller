
#include<avr/io.h>
#include<util/delay.h>
#include<stdio.h>

#define lcd_RS PB0
#define lcd_RW PC5
#define lcd_EN PB1

void lcd(void){
  PORTB |= (1<<lcd_EN);
  _delay_us(1);
  PORTB &= ~(1<<lcd_EN);
  _delay_us(100);
}
void lcd_sendbit(uint8_t data){
PORTD = (PORTD & 0x0F) | (data & 0xF0);
lcd();
}
void lcd_cmd(uint8_t cmd){
  PORTB &= ~(1<<lcd_RS);
  lcd_sendbit(cmd);
  lcd_sendbit(cmd << 4);
  _delay_ms(2);
}
void lcd_char(char data){
  PORTB |= (1<<lcd_RS);
  lcd_sendbit(data);
  lcd_sendbit(data << 4);
  _delay_us(100);
}
void lcd_string(const char *str){
  while(*str){
    lcd_char(*str++);
  }
}
void lcd_setcursor(uint8_t row, uint8_t col){
  if(row == 0){
    lcd_cmd(0x80 + col);
  } else {
    lcd_cmd(0xC0 + col);
  }
}
void lcd_init(void){
  DDRB |= (1<<lcd_RS) | (1<<lcd_EN);
  DDRD |= 0xF0;
  _delay_ms(20);

  lcd_sendbit(0x30);
  _delay_ms(5);
  lcd_sendbit(0x30);
  _delay_us(100);
  lcd_sendbit(0x30);
  lcd_sendbit(0x20);
  
  lcd_cmd(0x28);
  lcd_cmd(0x0C);
  lcd_cmd(0x01);
  _delay_ms(2);
}
void adc_init(void){
  ADMUX = (1<<REFS0);
  ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}
uint16_t adc_read(uint8_t channel){
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  ADCSRA |= (1<<ADSC);
  while(ADCSRA & (1<<ADSC));
  return ADC;
}
int main(void){
  DDRC |= (1<<lcd_RW);
  PORTC &= ~(1<<lcd_RW);
  _delay_ms(20);

  adc_init();
  lcd_init();

  lcd_setcursor(0, 0);
  lcd_string("Gas voltage:");

  uint16_t adc_value = 0;
  float voltage = 0.0;
  char buffer[17];

  while(1){
    adc_value = adc_read(0);
    voltage = (adc_value * 5.0) / 1023.0;

    int vInt  = (int)voltage;
    int vDec  = (int)((voltage - vInt) * 100);

    snprintf(buffer, sizeof(buffer), " %d.%02d V", vInt, vDec);

    lcd_setcursor(1, 0);
    lcd_string(buffer);
    _delay_ms(500);
  }
return 0;
}
