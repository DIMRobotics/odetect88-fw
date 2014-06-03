#include <avr/io.h>
#include <avr/interrupt.h>
#include <arch/antares.h>

#include <odetect/odetect.h>
#include <odetect/uart.h>

static volatile enum {
        MODE_IDLE = 0,
        MODE_LIMITS
} mode;

ANTARES_INIT_LOW(uart_init)
{
        UCSR1B = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1);
        UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);
        /* set baudrate 57600 baud */
        UBRR1L = 16;
        UBRR1H = 0;
}

void uart_send(uint8_t ch) {
        while (!(UCSR1A & (1<<UDRE1)));
        UDR1 = ch;
}

static inline void set_mode(uint8_t ch)
{
        switch (ch) {
                case 'l':
                        mode = MODE_LIMITS;
                        break;
                default:
                        mode = MODE_IDLE;
        }
}

static inline void mode_limits(uint8_t ch)
{
        static volatile uint8_t i = 0;
        
        odetect_set_limit(i, ch);
        i++;
        
        if (i == 6) {
                mode = MODE_IDLE;
                i = 0;
        }
}

ISR(USART1_RX_vect)
{
        switch (mode) {
                case MODE_IDLE:
                        set_mode(UDR1);
                        break;
                case MODE_LIMITS:
                        mode_limits(UDR1);
                        break;
        }
}
