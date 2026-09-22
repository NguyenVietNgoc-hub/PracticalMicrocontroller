#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define BAUD 9600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1) 
#define MIN_ANGLE 0
#define MAX_ANGLE 180


void UART_init(void) {
    
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);

    
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_transmitChar(char c) {
   
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void UART_transmitString(const char *str) {
    while (*str) {
        UART_transmitChar(*str++);
    }
}

bool UART_available(void) {
    return (UCSR0A & (1 << RXC0));
}

char UART_receiveChar(void) {
    while (!UART_available());
    return UDR0;
}


void Timer1_Servo_init(void) {
    
    DDRB |= (1 << PB1);

    
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); 

    
    ICR1 = 4999;
}

void setServoAngle(int angle) {
   
    uint16_t ocr_val = 125 + ((uint32_t)angle * 500) / 180;
    OCR1A = ocr_val;
}


void processCommand(const char *command) {
    char str[32];
    strncpy(str, command, sizeof(str) - 1);
    str[sizeof(str) - 1] = '\0';

    
    char *p = str;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

    
    int len = strlen(p);
    while (len > 0 && (p[len - 1] == ' ' || p[len - 1] == '\t' || p[len - 1] == '\r' || p[len - 1] == '\n')) {
        p[--len] = '\0';
    }

    if (len == 0) {
        UART_transmitString("ERROR: Please enter an angle.\r\n");
        return;
    }

    char *end;
    long angle = strtol(p, &end, 10);

    if (*end != '\0' || angle < MIN_ANGLE || angle > MAX_ANGLE) {
        UART_transmitString("ERROR: Please enter a valid integer angle.\r\n");
        return;
    }

    setServoAngle((int)angle);

    char response[64];
    snprintf(response, sizeof(response), "OK: Servo moved to %ld degrees.\r\n", angle);
    UART_transmitString(response);
}

void setup(void) {
    UART_init();
    Timer1_Servo_init();

    setServoAngle(90);
    UART_transmitString("Servo ready. Enter an angle from 0 to 180.\r\n");
}

char rxBuffer[32];
uint8_t rxIndex = 0;

void loop(void) {
    if (UART_available()) {
        char c = UART_receiveChar();
        if (c == '\n' || c == '\r') {
            if (rxIndex > 0) {
                rxBuffer[rxIndex] = '\0';
                processCommand(rxBuffer);
                rxIndex = 0;
            }
        } else {
            if (rxIndex < sizeof(rxBuffer) - 1) {
                rxBuffer[rxIndex++] = c;
            }
        }
    }
}


int main(void) {
    setup();
    while (1) {
        loop();
    }
    return 0;
}
