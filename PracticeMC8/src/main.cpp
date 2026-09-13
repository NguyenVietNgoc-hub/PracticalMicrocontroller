#include <Arduino.h>
#include <LiquidCrystal.h>
#include <avr/io.h>
#include <util/delay.h>

// 1. Khởi tạo LCD 16x2 (RS, EN, D4, D5, D6, D7)
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// 2. Định nghĩa chân I2C Bit-Banging
#define SDA_PIN  PC4  // Chân A4
#define SCL_PIN  PC5  // Chân A5

// 3. Định nghĩa các chân nguồn / tín hiệu giả lập trên Port B
#define VCC_PIN  PB0  // Chân D8  -> Giả lập VCC (5V) cho Cảm biến
#define GND_PIN  PB1  // Chân D9  -> Giả lập GND (0V) cho Cảm biến
#define RW_PIN   PB2  // Chân D10 -> Giả lập GND (0V) cho chân RW của LCD

// Địa chỉ I2C của Cảm biến nhiệt độ (0x38)
#define TEMP_SENSOR_ADDR 0x38 

// Macro thao tác Bit-Banging I2C bằng thanh ghi
#define SDA_HIGH()  do { DDRC &= ~(1 << SDA_PIN); PORTC |= (1 << SDA_PIN); } while (0)
#define SDA_LOW()   do { DDRC |= (1 << SDA_PIN);  PORTC &= ~(1 << SDA_PIN); } while (0)
#define SCL_HIGH()  do { DDRC &= ~(1 << SCL_PIN); PORTC |= (1 << SCL_PIN); } while (0)
#define SCL_LOW()   do { DDRC |= (1 << SCL_PIN);  PORTC &= ~(1 << SCL_PIN); } while (0)
#define READ_SDA()  ((PINC & (1 << SDA_PIN)) != 0)

// ============================================================================
// HÀM GIAO TIẾP I2C TỰ ĐỊNH NGHĨA (BIT-BANGING BẰNG THANH GHI)
// ============================================================================

void i2c_init() {
    // 1. Cấu hình D8 (VCC), D9 (GND) và D10 (RW) làm OUTPUT bằng thanh ghi DDRB
    DDRB |= (1 << VCC_PIN) | (1 << GND_PIN) | (1 << RW_PIN);

    // 2. Cài đặt mức điện áp:
    PORTB |= (1 << VCC_PIN);   // D8  = HIGH (5V)
    PORTB &= ~(1 << GND_PIN);  // D9  = LOW  (0V)
    PORTB &= ~(1 << RW_PIN);   // D10 = LOW  (0V) -> Giả lập GND nối chân RW LCD

    // 3. Khởi tạo bus I2C thả nổi HIGH
    SDA_HIGH();
    SCL_HIGH();
    _delay_ms(40); // Chờ cảm biến ổn định nguồn
}

void i2c_start() {
    SDA_HIGH(); SCL_HIGH(); _delay_us(4);
    SDA_LOW();  _delay_us(4);
    SCL_LOW();  _delay_us(4);
}

void i2c_stop() {
    SDA_LOW();  _delay_us(4);
    SCL_HIGH(); _delay_us(4);
    SDA_HIGH(); _delay_us(4);
}

uint8_t i2c_write(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x80) {
            SDA_HIGH();
        } else {
            SDA_LOW();
        }
        data <<= 1;
        _delay_us(2);
        SCL_HIGH(); _delay_us(2);
        SCL_LOW();  _delay_us(2);
    }
    SDA_HIGH(); _delay_us(2);
    SCL_HIGH(); _delay_us(2);
    uint8_t ack = (READ_SDA() == 0);
    SCL_LOW();  _delay_us(2);
    return ack;
}

uint8_t i2c_read_ack() {
    uint8_t data = 0;
    SDA_HIGH();
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        SCL_HIGH(); _delay_us(2);
        if (READ_SDA()) data |= 1;
        SCL_LOW();  _delay_us(2);
    }
    SDA_LOW();  _delay_us(2);
    SCL_HIGH(); _delay_us(2);
    SCL_LOW();  _delay_us(2);
    return data;
}

uint8_t i2c_read_nack() {
    uint8_t data = 0;
    SDA_HIGH();
    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        SCL_HIGH(); _delay_us(2);
        if (READ_SDA()) data |= 1;
        SCL_LOW();  _delay_us(2);
    }
    SDA_HIGH(); _delay_us(2);
    SCL_HIGH(); _delay_us(2);
    SCL_LOW();  _delay_us(2);
    return data;
}

// ============================================================================
// HÀM ĐỌC CẢM BIẾN NHIỆT ĐỘ (ĐỊA CHỈ 0x38)
// ============================================================================

bool init_temperature_sensor() {
    // AHT10/AHT20 cần lệnh khởi tạo calibration trước lần đo đầu tiên.
    i2c_start();
    bool acknowledged = i2c_write(TEMP_SENSOR_ADDR << 1);
    acknowledged = i2c_write(0xE1) && acknowledged;
    acknowledged = i2c_write(0x08) && acknowledged;
    acknowledged = i2c_write(0x00) && acknowledged;
    i2c_stop();
    _delay_ms(10);
    return acknowledged;
}

// Hàm gửi lệnh đo và đọc dữ liệu từ cảm biến nhiệt độ I2C
int16_t read_temperature_sensor() {
    // 1. Gửi lệnh kích hoạt đo (Trigger Measurement: 0xAC, 0x33, 0x00)
    i2c_start();
    bool acknowledged = i2c_write(TEMP_SENSOR_ADDR << 1); // Ghi (R/W = 0)
    acknowledged = i2c_write(0xAC) && acknowledged;
    acknowledged = i2c_write(0x33) && acknowledged;
    acknowledged = i2c_write(0x00) && acknowledged;
    i2c_stop();

    if (!acknowledged) {
        return -127;
    }

    _delay_ms(80); // Đợi cảm biến hoàn tất đo

    // 2. Đọc dữ liệu trả về từ cảm biến
    i2c_start();
    if (!i2c_write((TEMP_SENSOR_ADDR << 1) | 1)) { // Đọc (R/W = 1)
        i2c_stop();
        return -127;
    }
    
    uint8_t state = i2c_read_ack();
    i2c_read_ack();
    i2c_read_ack();
    uint8_t data3 = i2c_read_ack();
    uint8_t data4 = i2c_read_ack();
    uint8_t data5 = i2c_read_nack(); // Byte cuối gửi NACK
    i2c_stop();

    if (state & 0x80) {
        return -127;
    }

    // Tính toán nhiệt độ (°C), giữ kiểu có dấu để không làm sai nhiệt độ âm.
    uint32_t raw_temp = (((uint32_t)(data3 & 0x0F)) << 16) | (((uint32_t)data4) << 8) | data5;
    int16_t temp_c = (int16_t)((raw_temp * 200UL) / 1048576UL) - 50;

    return temp_c;
}

// ============================================================================
// CHƯƠNG TRÌNH CHÍNH
// ============================================================================

void setup() {
    // Khởi tạo các chân GPIO/Nguồn giả lập và giao thức I2C trước
    i2c_init();
    init_temperature_sensor();

    // Khởi tạo LCD 16x2
    lcd.begin(16, 2);
    lcd.print("Temp Sensor I2C");
    delay(1000);
    lcd.clear();
}

void loop() {
    // Đọc nhiệt độ từ cảm biến I2C (0x38)
    int16_t temp = read_temperature_sensor();

    // Hiển thị lên LCD 16x2
    lcd.setCursor(0, 0);
    lcd.print("Temp (0x38):");
    
    lcd.setCursor(0, 1);
    lcd.print("     "); // Xóa ký tự cũ
    lcd.setCursor(0, 1);
    if (temp == -127) {
        lcd.print("Sensor error");
    } else {
        lcd.print(temp);
        lcd.print(" C");
    }

    delay(1000); // Cập nhật mỗi giây
}