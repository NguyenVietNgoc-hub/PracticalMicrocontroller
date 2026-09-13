#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#define BAUD 9600
 
volatile uint16_t pulse_start  = 0;  
volatile uint32_t pulse_length = 0;  
volatile uint8_t  new_data     = 0;  

ISR(TIMER1_CAPT_vect)
{
    uint16_t now = ICR1;   
 
    if (TCCR1B & (1 << ICES1)) {
        pulse_start = now;
        TCCR1B &= ~(1 << ICES1);   
    } else {
        pulse_length = now - pulse_start;
        new_data = 1;               
        TCCR1B |= (1 << ICES1);     
    }
}
 
void timer1_setup(void)
{
    DDRB &= ~(1 << DDB0);   
 
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << ICNC1);              
    TCCR1B |= (1 << ICES1);             
    TCCR1B |= (1 << CS12) | (1 << CS10); 
 
    TIMSK1 |= (1 << ICIE1);   
 
    TCNT1 = 0;   
}
 
void uart_setup(void)
{
    uint16_t ubrr = F_CPU / 16 / BAUD - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
 
void uart_send_char(char c)
{
    while (!(UCSR0A & (1 << UDRE0))) {}   
    UDR0 = c;
}
 
void uart_send_text(const char *s)
{
    while (*s) uart_send_char(*s++);
}
 

int main(void)
{
    char msg[64];
 
    uart_setup();
    timer1_setup();
    sei();   
 
    uart_send_text("Waiting for motion...\r\n");
 
    while (1) {
        cli();
        uint8_t got_data = new_data;
        uint32_t ticks = pulse_length;
        if (got_data) new_data = 0;
        sei();
 
        if (got_data) {
            float seconds = ticks * 0.000064f;   
            snprintf(msg, sizeof(msg), "Motion lasted: %.2f seconds\r\n", seconds);
            uart_send_text(msg);
        }
    }
}
