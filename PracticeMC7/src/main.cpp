#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#define BAUD 9600
 
// These 3 boxes are shared between the "phone call" (interrupt) and
// main(). "volatile" just means: "this can change at any moment,
// don't assume it stays the same".
volatile uint16_t pulse_start  = 0;  // clock reading when PIR turned ON
volatile uint32_t pulse_length = 0;  // how long PIR stayed ON (in ticks)
volatile uint8_t  new_data     = 0;  // 1 = "hey main(), I have a result for you!"
 
// -----------------------------------------------------------------
// This function runs AUTOMATICALLY every time the PIR wire changes
// (from OFF to ON, or from ON to OFF). Think of it as a phone that
// rings by itself whenever something happens - you don't call it,
// the chip calls it for you.
// -----------------------------------------------------------------
ISR(TIMER1_CAPT_vect)
{
    uint16_t now = ICR1;   // the clock reading the chip just took a photo of
 
    if (TCCR1B & (1 << ICES1)) {
        // We were waiting for PIR to turn ON, and it just did.
        // This is like pressing "Start" on the stopwatch.
        pulse_start = now;
        TCCR1B &= ~(1 << ICES1);   // now watch for PIR turning OFF next
    } else {
        // We were waiting for PIR to turn OFF, and it just did.
        // This is like pressing "Stop" and reading the time.
        pulse_length = now - pulse_start;
        new_data = 1;               // tell main() "go read the result!"
        TCCR1B |= (1 << ICES1);     // now watch for PIR turning ON next
    }
}
 
// -----------------------------------------------------------------
// One-time setup: get the "stopwatch" ready
// -----------------------------------------------------------------
void timer1_setup(void)
{
    DDRB &= ~(1 << DDB0);   // pin PB0 = input (this is where PIR connects)
 
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << ICNC1);              // ignore tiny noise glitches
    TCCR1B |= (1 << ICES1);              // start: watch for PIR turning ON
    TCCR1B |= (1 << CS12) | (1 << CS10); // slow the clock down (1 tick = 64 microseconds)
 
    TIMSK1 |= (1 << ICIE1);   // "please call ISR whenever the PIR wire changes"
 
    TCNT1 = 0;   // reset the clock to 0
}
 
// -----------------------------------------------------------------
// UART - just so we can print results to the computer screen
// -----------------------------------------------------------------
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
    while (!(UCSR0A & (1 << UDRE0))) {}   // wait until ready to send
    UDR0 = c;
}
 
void uart_send_text(const char *s)
{
    while (*s) uart_send_char(*s++);
}
 
// -----------------------------------------------------------------
// MAIN - runs forever
// -----------------------------------------------------------------
int main(void)
{
    char msg[64];
 
    uart_setup();
    timer1_setup();
    sei();   // turn on interrupts - WITHOUT THIS LINE, NOTHING WORKS AT ALL
 
    uart_send_text("Waiting for motion...\r\n");
 
    while (1) {
        // Reading pulse_length safely: since it's a "big" number (32-bit)
        // and the chip is only 8-bit, reading it takes more than one step.
        // If the interrupt fires in the middle of that, we could read a
        // half-updated (wrong) value. So we briefly pause interrupts,
        // grab a clean copy, then turn interrupts back on.
        cli();
        uint8_t got_data = new_data;
        uint32_t ticks = pulse_length;
        if (got_data) new_data = 0;
        sei();
 
        if (got_data) {
            float seconds = ticks * 0.000064f;   // 1 tick = 64 microseconds
            snprintf(msg, sizeof(msg), "Motion lasted: %.2f seconds\r\n", seconds);
            uart_send_text(msg);
        }
    }
}