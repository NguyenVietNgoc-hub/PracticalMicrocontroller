#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)


void USART_Init(unsigned int ubrr) {
  
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)(ubrr);
    
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void USART_TransmitChar(char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void USART_SendString(const char *str) {
    while (*str) {
        USART_TransmitChar(*str++);
    }
}

int main(void) {
    DDRD &= ~(1 << PD2);

    DDRB |= (1 << PB5) | (1 << PB4);

    USART_Init(UBRR_VAL);

    uint8_t previous_state = 0xFF;

    while (1) {
        uint8_t current_state = (PIND & (1 << PD2)) ? 1 : 0;

        if (current_state != previous_state) {
            if (current_state == 1) {
                PORTB |= (1 << PB5) | (1 << PB4);
                USART_SendString("Motion Detected!\r\n");
            } else {
                PORTB &= ~((1 << PB5) | (1 << PB4));
                USART_SendString("No Motion.\r\n");
            }

            previous_state = current_state;
        }

        _delay_ms(50);
    }

    return 0;
}
