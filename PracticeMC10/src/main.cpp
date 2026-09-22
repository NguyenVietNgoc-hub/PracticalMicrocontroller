#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

// Initialize Hardware USART0
void USART_Init(unsigned int ubrr) {
    // Set baud rate registers
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)(ubrr);
    
    // Enable Transmitter (TXEN0)
    UCSR0B = (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit, no parity (8N1)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Send a single character over UART
void USART_TransmitChar(char data) {
    // Wait until transmit buffer is empty (UDRE0 bit set)
    while (!(UCSR0A & (1 << UDRE0)));
    // Load data into buffer
    UDR0 = data;
}

// Send a string over UART
void USART_SendString(const char *str) {
    while (*str) {
        USART_TransmitChar(*str++);
    }
}

int main(void) {
    // Configure PD2 as INPUT for PIR Sensor
    DDRD &= ~(1 << PD2);

    // Configure PB5 as OUTPUT for LED and PB4 as OUTPUT for buzzer
    DDRB |= (1 << PB5) | (1 << PB4);

    // Initialize USART hardware module
    USART_Init(UBRR_VAL);

    // Track previous sensor state to avoid repeating identical serial messages
    // Set to 0xFF initially so the first reading triggers a state change on boot
    uint8_t previous_state = 0xFF;

    while (1) {
        // Read digital signal from PIR sensor pin (PD2 in Input Pins Register D)
        uint8_t current_state = (PIND & (1 << PD2)) ? 1 : 0;

        // Check if sensor state has changed
        if (current_state != previous_state) {
            if (current_state == 1) {
                // Motion Detected: turn ON LED on PB5 and buzzer on PB4
                PORTB |= (1 << PB5) | (1 << PB4);
                USART_SendString("Motion Detected!\r\n");
            } else {
                // No Motion: turn OFF LED on PB5 and buzzer on PB4
                PORTB &= ~((1 << PB5) | (1 << PB4));
                USART_SendString("No Motion.\r\n");
            }

            // Save new state
            previous_state = current_state;
        }

        _delay_ms(50); // Polling delay
    }

    return 0;
}