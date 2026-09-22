#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define DARK_THRESHOLD 500 

const uint8_t seg_code[10] = {
    0xC0, // 0: 1100 0000
    0xF9, // 1: 1111 1001
    0xA4, // 2: 1010 0100
    0xB0, // 3: 1011 0000
    0x99, // 4: 1001 1001
    0x92, // 5: 1001 0010
    0x82, // 6: 1000 0010
    0xF8, // 7: 1111 1000
    0x80, // 8: 1000 0000
    0x90  // 9: 1001 0000
};

volatile uint8_t display_digits[4] = {0, 0, 0, 0};
volatile uint8_t current_digit = 0;

void IO_Init(void) {
   
    DDRC |= (1 << PC2);
    PORTC &= ~(1 << PC2); 
    
    DDRD |= 0xFC; 
    DDRB |= (1 << PB0);
    
 
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);
    
  
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);

void ADC_Init(void) {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void UART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Timer0_Init(void) {
    TCCR0B = (1 << CS01) | (1 << CS00);
    TIMSK0 = (1 << TOIE0);
}

uint16_t ADC_Read(uint8_t channel) {
    channel &= 0b00000111;
    ADMUX = (ADMUX & 0xF8) | channel;
    ADCSRA |= (1 << ADSC);
    while(ADCSRA & (1 << ADSC));
    return (ADC);
}

void UART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void UART_SendString(const char* str) {
    while (*str) {
        UART_Transmit(*str++);
    }
}

void UART_SendInteger(uint16_t num) {
    char buffer[6];
    itoa(num, buffer, 10);
    UART_SendString(buffer);
}


void UpdateDisplayBuffer(uint16_t value) {
    if (value > 9999) value = 9999;
    display_digits[3] = value % 10;           
    display_digits[2] = (value / 10) % 10;    
    display_digits[1] = (value / 100) % 10;   
    display_digits[0] = (value / 1000) % 10;  
}


ISR(TIMER0_OVF_vect) {
   
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);
    uint8_t code = seg_code[display_digits[current_digit]];
    PORTD = (PORTD & 0x03) | ((code & 0x3F) << 2);
    
    if (code & (1 << 6)) {
        PORTB |= (1 << PB0);
    } else {
        PORTB &= ~(1 << PB0);
    }
    
    PORTB &= ~(1 << (PB1 + current_digit));
    
    current_digit++;
    if (current_digit >= 4) current_digit = 0;
}


int main(void) {
    uint16_t adc_value;
    
    
    IO_Init();
    ADC_Init();
    UART_Init(103); 
    Timer0_Init();
    
    sei();
    
    UART_SendString("==== LDR ADC SYSTEM INITIALIZED ====\r\n");

    while (1) {
        adc_value = ADC_Read(0);
        UpdateDisplayBuffer(adc_value);
        
        UART_SendString("ADC Value: ");
        UART_SendInteger(adc_value);
        UART_SendString(" - Condition: ");
        
        if (adc_value < DARK_THRESHOLD) {
            PORTC |= (1 << PC2); 
            UART_SendString("DARK (LED ON)\r\n");
        } else {
            PORTC &= ~(1 << PC2); 
            UART_SendString("BRIGHT (LED OFF)\r\n");
        }
        
        _delay_ms(500);
    }
    return 0;
}
