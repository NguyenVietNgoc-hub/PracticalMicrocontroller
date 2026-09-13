

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

volatile uint32_t system_ticks = 0;
uint16_t adc_val = 0;
uint8_t servo_pos = 0;


void UART_Init(void) {
    uint16_t ubrr = (F_CPU / (16UL * 9600UL)) - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    DDRD |= (1 << PD1);
    UCSR0B = (1 << TXEN0); 
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); 
}

void UART_SendChar(char c) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void UART_SendString(const char *str) {
    while (*str) UART_SendChar(*str++);
}


void ADC_Init(void) {
    ADMUX = (1 << REFS0); 
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t ADC_Read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}


void Timer0_Init(void) {
    TCCR0A = (1 << WGM01);              
    TCCR0B = (1 << CS01) | (1 << CS00); 
    OCR0A = 249;                        
    TIMSK0 |= (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
    system_ticks++;
}


void Timer1_Init(void) {
    DDRB |= (1 << PB1) | (1 << PB2);
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12); 
    ICR1 = 1249;
}


void Task1_ReadADC(void) {
    adc_val = ADC_Read(0); 
}

void Task2_UpdatePWM(void) {
  
    servo_pos = (servo_pos + 5) % 180;
    uint16_t pulse = 31 + ((uint32_t)servo_pos * (156 - 31)) / 180;
    OCR1A = pulse;
    OCR1B = pulse;
}

void Task3_ReadSensor(void) {
}

void Task4_SendUART(void) {
    char buf[50];
    sprintf(buf, "ADC: %d | Servo Angle: %d deg\r\n", adc_val, servo_pos);
    UART_SendString(buf);
}

int main(void) {
    UART_Init();
    UART_SendString("UART ready\r\n");
    ADC_Init();
    Timer0_Init();
    Timer1_Init();
    
    sei(); 

    uint32_t t1 = 0, t2 = 0, t3 = 0, t4 = 0;

    while (1) {
        uint32_t now = system_ticks;

        if (now - t1 >= 10)   { t1 = now; Task1_ReadADC(); }
        if (now - t2 >= 20)   { t2 = now; Task2_UpdatePWM(); }
        if (now - t3 >= 100)  { t3 = now; Task3_ReadSensor(); }
        if (now - t4 >= 1000) { t4 = now; Task4_SendUART(); }
    }
}
