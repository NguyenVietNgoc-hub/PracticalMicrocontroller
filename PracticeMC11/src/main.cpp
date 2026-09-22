

// #include <avr/io.h>
// #include <util/delay.h>
// #include <stdio.h>

// #define BAUD 9600
// #define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)
// #define DARK_THRESHOLD 500  // ADC threshold (0-1023); lower value = darker

// // 7-Segment digit patterns for Common Cathode (Segments A-F on PD2-PD7, Segment G on PB0)
// // Bits 0..5 = Segments A..F, Bit 6 = Segment G
// const uint8_t seg_patterns[10] = {
//     0b00111111, // 0
//     0b00000110, // 1
//     0b01011011, // 2
//     0b01001111, // 3
//     0b01100110, // 4
//     0b01101101, // 5
//     0b01111101, // 6
//     0b00000111, // 7
//     0b01111111, // 8
//     0b01101111  // 9
// };

// // Initialize USART hardware module
// void UART_Init(unsigned int ubrr) {
//     UBRR0H = (unsigned char)(ubrr >> 8);
//     UBRR0L = (unsigned char)(ubrr);
//     UCSR0B = (1 << TXEN0);                         // Enable Transmitter
//     UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);       // 8 Data, 1 Stop, No Parity
// }

// void UART_TransmitChar(char data) {
//     while (!(UCSR0A & (1 << UDRE0)));
//     UDR0 = data;
// }

// void UART_SendString(const char *str) {
//     while (*str) {
//         UART_TransmitChar(*str++);
//     }
// }

// // Initialize Analog-to-Digital Converter
// void ADC_Init(void) {
//     // Select AVcc with external capacitor at AREF pin, Channel 0 (PC0)
//     ADMUX = (1 << REFS0);
    
//     // Enable ADC and set prescaler to 128 (16MHz / 128 = 125kHz ADC clock)
//     ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
// }

// // Read 10-bit analog voltage conversion from specified ADC channel
// uint16_t ADC_Read(uint8_t channel) {
//     // Select ADC channel (0-7)
//     ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    
//     // Start conversion
//     ADCSRA |= (1 << ADSC);
    
//     // Wait for conversion to complete
//     while (ADCSRA & (1 << ADSC));
    
//     // Return 10-bit ADC value
//     return ADC;
// }

// // Output a digit onto 7-segment display pins for Common Anode display
// void SevenSeg_SetPattern(uint8_t digit) {
//     uint8_t pattern = (~seg_patterns[digit % 10]) & 0x7F;

//     // Turn off all segments first
//     PORTD |= 0xFC;
//     PORTB |= (1 << PB0);

//     // Set segments A-F (PD2-PD7) to LOW for ON segments, HIGH for OFF segments
//     PORTD &= ~((pattern & 0x3F) << 2);

//     // Set segment G (PB0)
//     if (pattern & 0x40) {
//         PORTB &= ~(1 << PB0);
//     }
// }

// // Display 4-digit number on multiplexed 7-segment display for Common Anode
// void SevenSeg_DisplayNumber(uint16_t num) {
//     uint8_t digits[4];
//     digits[0] = (num / 1000) % 10;
//     digits[1] = (num / 100) % 10;
//     digits[2] = (num / 10) % 10;
//     digits[3] = num % 10;

//     uint8_t digit_pins[4] = {PC1, PC2, PC3, PC4};

//     for (uint8_t i = 0; i < 4; i++) {
//         // Common Anode: all digit selectors OFF at LOW level
//         PORTC &= ~((1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4));

//         // Output segment patterns
//         SevenSeg_SetPattern(digits[i]);

//         // Enable current digit by setting the common anode HIGH
//         PORTC |= (1 << digit_pins[i]);

//         _delay_ms(2); // Refresh delay per digit
//     }
// }

// int main(void) {
//     // Configure PB5 (LED) and PB0 (Seg G) as OUTPUT
//     DDRB |= (1 << PB5) | (1 << PB0);
    
//     // Configure PD2..PD7 (Seg A..F) as OUTPUT
//     DDRD |= 0xFC;
    
//     // Configure PC1..PC4 (Digit Selects 1..4) as OUTPUT, PC0 as INPUT (LDR)
//     DDRC |= (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4);
//     DDRC &= ~(1 << PC0);

//     UART_Init(UBRR_VAL);
//     ADC_Init();

//     char uart_buffer[40];
//     uint16_t adc_value = 0;
//     uint16_t display_value = 0;
//     uint8_t sample_tick = 0;

//     while (1) {
//         // Read ADC continuously for LED decision (independent from 7-seg refresh)
//         adc_value = ADC_Read(0);

//         // Turn ON LED when environment is dark, turn OFF when bright
//         if (adc_value < DARK_THRESHOLD) {
//             PORTB |= (1 << PB5);  // LED ON (Dark)
//         } else {
//             PORTB &= ~(1 << PB5); // LED OFF (Bright)
//         }

//         // Update the display value every ~3 seconds
//         if (++sample_tick >= 100) {
//             sample_tick = 0;
//             display_value = adc_value;

//             // Send ADC value & light status over UART periodically (~every 3s)
//             const char *status = (adc_value < DARK_THRESHOLD) ? "Dark" : "Bright";
//             snprintf(uart_buffer, sizeof(uart_buffer), "ADC: %u | Status: %s\r\n", adc_value, status);
//             UART_SendString(uart_buffer);
//         }

//         // Multiplex 7-segment display to show the value updated every 3 seconds
//         SevenSeg_DisplayNumber(display_value);

//         _delay_ms(30);
//     }

//     return 0;
// }
#ifndef F_CPU
#define F_CPU 16000000UL // Giả định thạch anh 16MHz
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// --- Định nghĩa ngưỡng ánh sáng ---
// Giả định qua đo đạc thực tế: Môi trường sáng ADC > 500, tối ADC < 300. 
// Ta chọn ngưỡng là 400 để phân định sáng/tối.
#define DARK_THRESHOLD 500 

// --- Bảng mã LED 7 đoạn Anode chung (0 là sáng, 1 là tắt) ---
// Thứ tự bit: dp-g-f-e-d-c-b-a
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

// Mảng chứa các chữ số cần hiển thị
volatile uint8_t display_digits[4] = {0, 0, 0, 0};
volatile uint8_t current_digit = 0;

// ================= HÀM KHỞI TẠO =================

void IO_Init(void) {
    // LED cảnh báo ở PC2 (Output)
    DDRC |= (1 << PC2);
    PORTC &= ~(1 << PC2); // Ban đầu tắt
    
    // Các chân Segment: PD2-PD7 (a-f), PB0 (g) -> Output
    DDRD |= 0xFC; // 0b11111100 (Chân PD2 đến PD7)
    DDRB |= (1 << PB0);
    
    // Các chân điều khiển số (Digit 1-4): PB1-PB4 -> Output
    DDRB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);
    
    // Khởi tạo tắt hết LED 7 đoạn (Anode chung: tắt Segment = 1, hoặc tắt Digit = 1)
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4); // Tắt các digit
}

void ADC_Init(void) {
    // Chọn Vref = AVCC, kênh ADC0 (MUX3:0 = 0000)
    ADMUX = (1 << REFS0);
    
    // Enable ADC (ADEN), Prescaler = 128 (ADPS2:0 = 111)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

void UART_Init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    
    // Bật bộ truyền và bộ nhận
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Frame: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Timer0_Init(void) {
    // Normal mode, Prescaler = 64
    TCCR0B = (1 << CS01) | (1 << CS00);
    // Bật ngắt tràn Timer0
    TIMSK0 = (1 << TOIE0);
}

// ================= HÀM CHỨC NĂNG =================

uint16_t ADC_Read(uint8_t channel) {
    // Đảm bảo channel nằm trong khoảng 0-7
    channel &= 0b00000111;
    // Xóa cài đặt kênh cũ và ghi kênh mới vào thanh ghi ADMUX
    ADMUX = (ADMUX & 0xF8) | channel;
    
    // Bắt đầu chuyển đổi
    ADCSRA |= (1 << ADSC);
    
    // Chờ quá trình chuyển đổi hoàn tất (cờ ADSC bị xóa tự động về 0)
    while(ADCSRA & (1 << ADSC));
    
    // Theo datasheet, đọc ADCL trước, ADCH sau (thanh ghi ADC 16-bit macro tự làm việc này)
    return (ADC);
}

void UART_Transmit(unsigned char data) {
    // Chờ bộ đệm phát trống
    while (!(UCSR0A & (1 << UDRE0)));
    // Ghi dữ liệu vào thanh ghi đệm UDR0
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

// Tách các chữ số của một số nguyên lưu vào mảng hiển thị
void UpdateDisplayBuffer(uint16_t value) {
    // Nếu giá trị lớn hơn 9999, giới hạn lại (dù ADC max 1023)
    if (value > 9999) value = 9999;
    display_digits[3] = value % 10;           // Hàng đơn vị
    display_digits[2] = (value / 10) % 10;    // Hàng chục
    display_digits[1] = (value / 100) % 10;   // Hàng trăm
    display_digits[0] = (value / 1000) % 10;  // Hàng nghìn
}

// ================= NGẮT TIMER0 =================

// Ngắt tràn Timer0: Thực hiện quét LED 7-đoạn (Multiplexing)
ISR(TIMER0_OVF_vect) {
    // 1. Tắt tất cả các Digit để tránh hiệu ứng bóng mờ (Ghosting)
    PORTB |= (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4);
    
    // 2. Xuất mã Segment ra PORTD và PB0
    uint8_t code = seg_code[display_digits[current_digit]];
    
    // Cập nhật bit 2 đến 7 của PORTD (tương ứng a-f). Xóa bit cũ, ghi bit mới.
    // Dịch mã code sang trái 2 bit để khớp phần cứng.
    PORTD = (PORTD & 0x03) | ((code & 0x3F) << 2);
    
    // Cập nhật PB0 (tương ứng đoạn g, bit 6 của mã code)
    if (code & (1 << 6)) {
        PORTB |= (1 << PB0);
    } else {
        PORTB &= ~(1 << PB0);
    }
    
    // 3. Bật Digit hiện tại (Kéo LOW)
    PORTB &= ~(1 << (PB1 + current_digit));
    
    // 4. Chuyển sang Digit tiếp theo cho chu kỳ ngắt sau
    current_digit++;
    if (current_digit >= 4) current_digit = 0;
}

// ================= CHƯƠNG TRÌNH CHÍNH =================

int main(void) {
    uint16_t adc_value;
    
    // Khởi tạo các module
    IO_Init();
    ADC_Init();
    UART_Init(103); // 103 cho Baudrate 9600 tại 16MHz
    Timer0_Init();
    
    // Bật ngắt toàn cục
    sei();
    
    UART_SendString("==== LDR ADC SYSTEM INITIALIZED ====\r\n");

    while (1) {
        // Đọc giá trị ADC từ kênh 0 (PC0)
        adc_value = ADC_Read(0);
        
        // Cập nhật giá trị ra LED 7 đoạn
        UpdateDisplayBuffer(adc_value);
        
        // Gửi qua UART
        UART_SendString("ADC Value: ");
        UART_SendInteger(adc_value);
        UART_SendString(" - Condition: ");
        
        // Xử lý logic bật/tắt LED cảnh báo và UART message
        if (adc_value < DARK_THRESHOLD) {
            PORTC |= (1 << PC2); // Bật LED (Trời tối)
            UART_SendString("DARK (LED ON)\r\n");
        } else {
            PORTC &= ~(1 << PC2); // Tắt LED (Trời sáng)
            UART_SendString("BRIGHT (LED OFF)\r\n");
        }
        
        // Trễ 0.5s giữa các lần gửi UART. 
        // Lệnh delay ở đây KHÔNG làm tắt 7-đoạn vì 7-đoạn được duy trì bởi ngắt Timer0
        _delay_ms(500);
    }
    return 0;
}