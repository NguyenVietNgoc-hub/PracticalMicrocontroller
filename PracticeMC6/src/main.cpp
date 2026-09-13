#include <avr/io.h>
#include <util/delay.h>

#define STEP_PIN PD3
#define DIR_PIN  PD4
#define EN_PIN   PD2

#define STEPS_PER_REV 200   

void stepper_init(void)
{
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN) | (1 << EN_PIN);

    PORTD &= ~(1 << EN_PIN);    
    PORTD &= ~(1 << STEP_PIN); 

void stepper_set_direction(uint8_t direction)
{
    if (direction) {
        PORTD |= (1 << DIR_PIN);
    } else {
        PORTD &= ~(1 << DIR_PIN);
    }
}

void stepper_pulse_once(void)
{
    PORTD |= (1 << STEP_PIN);
    _delay_us(3);
    PORTD &= ~(1 << STEP_PIN);
    _delay_us(3);
}


#define COUNTERCLOCKWISE_BUTTON_PIN PB2
#define STOP_BUTTON_PIN             PB3

void buttons_init(void)
{
    DDRB &= ~((1 << CLOCKWISE_BUTTON_PIN) |
              (1 << COUNTERCLOCKWISE_BUTTON_PIN) |
              (1 << STOP_BUTTON_PIN));
    PORTB |= (1 << CLOCKWISE_BUTTON_PIN) |
             (1 << COUNTERCLOCKWISE_BUTTON_PIN) |
             (1 << STOP_BUTTON_PIN);
}

uint8_t button_is_pressed(uint8_t pin)
{
    return !(PINB & (1 << pin));
}

int main(void)
{
    stepper_init();
    buttons_init();

    while (1) {
        if (button_is_pressed(STOP_BUTTON_PIN)) {
            PORTD &= ~(1 << STEP_PIN);
            _delay_ms(5);
            continue;
        }

        if (button_is_pressed(CLOCKWISE_BUTTON_PIN)) {
            stepper_set_direction(1);
            stepper_pulse_once();
            _delay_ms(2);
        } else if (button_is_pressed(COUNTERCLOCKWISE_BUTTON_PIN)) {
            stepper_set_direction(0);
            stepper_pulse_once();
            _delay_ms(2);
        } else {
            PORTD &= ~(1 << STEP_PIN);
            _delay_ms(5);
        }
    }
}
