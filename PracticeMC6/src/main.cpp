/*
 * Practice 6 - Control a stepper motor through a DRV8825 driver
 * Motor: 17HS4023 (NEMA17), 1.8 degrees per step -> 200 steps per revolution
 * Driver interface: just 3 signals - STEP (Pul), DIR (Dir), ENABLE (En)
 *
 * Wiring used in this example (change if you wired different pins):
 *   PD3 -> Pul (STEP)
 *   PD4 -> Dir (DIRECTION)
 *   PD2 -> En  (ENABLE)
 *   PB1 -> hold to rotate clockwise
 *   PB2 -> hold to rotate counter-clockwise
 *   PB3 -> stop
 *
 * DIP switches M0/M1/M2 on the driver: set all to OFF (full step mode)
 * so 200 pulses = exactly one full turn.
 */

#include <avr/io.h>
#include <util/delay.h>

// #define F_CPU 16000000UL

#define STEP_PIN PD3
#define DIR_PIN  PD4
#define EN_PIN   PD2

#define STEPS_PER_REV 200   // full step mode, 1.8 deg/step motor

// -----------------------------------------------------------------
// Set up the 3 pins as outputs and turn the driver ON.
// Most DRV8825 boards enable the motor when EN is LOW (active-low).
// If the motor stays stiff/silent and never turns, try flipping this
// (change the line below to set EN_PIN HIGH instead).
// -----------------------------------------------------------------
void stepper_init(void)
{
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN) | (1 << EN_PIN);

    PORTD &= ~(1 << EN_PIN);    // EN = LOW -> driver enabled (motor can move)
    PORTD &= ~(1 << STEP_PIN);  // start with STEP low
}

// -----------------------------------------------------------------
// Choose which way to turn.
// direction = 1 -> one way, direction = 0 -> the other way
// (which physical way is "clockwise" depends on how the motor wires
// are connected - just try it and see, then flip if it's backwards)
// -----------------------------------------------------------------
void stepper_set_direction(uint8_t direction)
{
    if (direction) {
        PORTD |= (1 << DIR_PIN);
    } else {
        PORTD &= ~(1 << DIR_PIN);
    }
}

// -----------------------------------------------------------------
// Send exactly ONE step pulse (one rising edge on STEP).
// The driver needs the pulse to stay high for at least ~2 microseconds -
// this short delay makes sure that requirement is met.
// -----------------------------------------------------------------
void stepper_pulse_once(void)
{
    PORTD |= (1 << STEP_PIN);
    _delay_us(3);
    PORTD &= ~(1 << STEP_PIN);
    _delay_us(3);
}

// -----------------------------------------------------------------
// Button setup. Three pushbuttons, all wired to GND:
//   PB1: hold to rotate clockwise
//   PB2: hold to rotate counter-clockwise
//   PB3: press to stop
// Internal pull-ups are enabled, so each pin reads HIGH normally and
// LOW when its button connects it to GND (pressed).
// -----------------------------------------------------------------
#define CLOCKWISE_BUTTON_PIN       PB1
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
        // STOP has priority over both direction buttons.
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