#include <Arduino.h>
#include <LiquidCrystal.h>
#include <avr/io.h>
#include <util/delay.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

#define SDA_PIN  PC4  // Chân A4
#define SCL_PIN  PC5  // Chân A5

#define VCC_PIN  PB0  // Chân D8  -> Giả lập VCC (5V) cho Cảm biến
#define GND_PIN  PB1  // Chân D9  -> Giả lập GND (0V) cho Cảm biến
#define RW_PIN   PB2  // Chân D10 -> Giả lập GND (0V) cho chân RW của LCD

#define TEMP_SENSOR_ADDR 0x38 

#define SDA_HIGH()  do { DDRC &= ~(1 << SDA_PIN); PORTC |= (1 << SDA_PIN); } while (0)
#define SDA_LOW()   do { DDRC |= (1 << SDA_PIN);  PORTC &= ~(1 << SDA_PIN); } while (0)
#define SCL_HIGH()  do { DDRC &= ~(1 << SCL_PIN); PORTC |= (1 << SCL_PIN); } while (0)
#define SCL_LOW()   do { DDRC |= (1 << SCL_PIN);  PORTC &= ~(1 << SCL_PIN); } while (0)
#define READ_SDA()  ((PINC & (1 << SDA_PIN)) != 0)


void i2c_init() {

    DDRB |= (1 << VCC_PIN) | (1 << GND_PIN) | (1 << RW_PIN);

    
    PORTB |= (1 << VCC_PIN);   
    PORTB &= ~(1 << GND_PIN);  
    PORTB &= ~(1 << RW_PIN);  

  
    SDA_HIGH();
    SCL_HIGH();
    _delay_ms(40); 
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

bool init_temperature_sensor() {
    i2c_start();
    bool acknowledged = i2c_write(TEMP_SENSOR_ADDR << 1);
    acknowledged = i2c_write(0xE1) && acknowledged;
    acknowledged = i2c_write(0x08) && acknowledged;
    acknowledged = i2c_write(0x00) && acknowledged;
    i2c_stop();
    _delay_ms(10);
    return acknowledged;
}

int16_t read_temperature_sensor() {
    i2c_start();
    bool acknowledged = i2c_write(TEMP_SENSOR_ADDR << 1); 
    acknowledged = i2c_write(0xAC) && acknowledged;
    acknowledged = i2c_write(0x33) && acknowledged;
    acknowledged = i2c_write(0x00) && acknowledged;
    i2c_stop();

    if (!acknowledged) {
        return -127;
    }

    _delay_ms(80); 
    
    i2c_start();
    if (!i2c_write((TEMP_SENSOR_ADDR << 1) | 1)) { 
        i2c_stop();
        return -127;
    }
    
    uint8_t state = i2c_read_ack();
    i2c_read_ack();
    i2c_read_ack();
    uint8_t data3 = i2c_read_ack();
    uint8_t data4 = i2c_read_ack();
    uint8_t data5 = i2c_read_nack(); 
    i2c_stop();

    if (state & 0x80) {
        return -127;
    }

    uint32_t raw_temp = (((uint32_t)(data3 & 0x0F)) << 16) | (((uint32_t)data4) << 8) | data5;
    int16_t temp_c = (int16_t)((raw_temp * 200UL) / 1048576UL) - 50;

    return temp_c;
}


void setup() {
    i2c_init();
    init_temperature_sensor();

    lcd.begin(16, 2);
    lcd.print("Temp Sensor I2C");
    delay(1000);
    lcd.clear();
}

void loop() {
    int16_t temp = read_temperature_sensor();

    lcd.setCursor(0, 0);
    lcd.print("Temp (0x38):");
    
    lcd.setCursor(0, 1);
    lcd.print("     "); 
    lcd.setCursor(0, 1);
    if (temp == -127) {
        lcd.print("Sensor error");
    } else {
        lcd.print(temp);
        lcd.print(" C");
    }

    delay(1000); 
}
