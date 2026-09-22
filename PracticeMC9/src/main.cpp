#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define BAUD 9600
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1) // UBRR = 103 cho Baudrate 9600

#define MIN_ANGLE 0
#define MAX_ANGLE 180

// --- Cấu hình USART (Serial) bằng Thanh ghi ---
void UART_init(void) {
    // Cài đặt tốc độ Baud (9600)
    UBRR0H = (uint8_t)(UBRR_VAL >> 8);
    UBRR0L = (uint8_t)(UBRR_VAL);

    // Bật chức năng nhận (RX) và truyền (TX)
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Cấu hình khung truyền: 8 bit dữ liệu, 1 bit stop, không parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_transmitChar(char c) {
    // Chờ đệm truyền trống
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

// --- Cấu hình Timer1 điều khiển Servo bằng PWM ---
void Timer1_Servo_init(void) {
    // Đặt chân PB1 (Digital Pin 9 trên Arduino) làm đầu ra (Output)
    DDRB |= (1 << PB1);

    // Cấu hình Fast PWM Mode 14 (TOP = ICR1)
    // Chế độ đảo ngược OC1A (Clear OC1A on Compare Match)
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); // Chia tần (Prescaler) = 64

    // Tần số PWM cho Servo là 50Hz (Chu kỳ 20ms)
    // Tần số Timer = 16MHz / 64 = 250kHz (mỗi tick = 4us)
    // ICR1 = (20ms / 4us) - 1 = 4999
    ICR1 = 4999;
}

void setServoAngle(int angle) {
    // Độ rộng xung điều khiển Servo tiêu chuẩn: 0.5ms đến 2.5ms (500us -> 2500us)
    // Giá trị OCR1A tương ứng:
    //  - 0 độ   => 500us / 4us = 125
    //  - 180 độ => 2500us / 4us = 625
    uint16_t ocr_val = 125 + ((uint32_t)angle * 500) / 180;
    OCR1A = ocr_val;
}

// --- Xử lý câu lệnh từ chuỗi Serial ---
void processCommand(const char *command) {
    char str[32];
    strncpy(str, command, sizeof(str) - 1);
    str[sizeof(str) - 1] = '\0';

    // Loại bỏ khoảng trắng / ký tự xuống dòng ở đầu chuỗi
    char *p = str;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;

    // Loại bỏ khoảng trắng ở cuối chuỗi
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

    setServoAngle(90); // Mặc định đặt góc 90 độ
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

// --- Điểm vào chương trình (bắt buộc khi build bằng avr-gcc thuần) ---
int main(void) {
    setup();
    while (1) {
        loop();
    }
    return 0;
}