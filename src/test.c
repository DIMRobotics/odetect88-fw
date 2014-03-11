#include <lib/cerebellum/gpio.h>
#include <lib/cerebellum/uart.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <lib/printk.h>

/**
 * PB7 - buzzer
 * GPC7 - channel 1 LED
 */

static uint8_t value;

int main(void)
{
        /* ADC confuguration testing */
        printk("ODetect-U4 debugging console\n\n");

        sei();

        GPIO_INIT_OUT(GPC7);

        while (1) {
                if (value > 80)
                        GPIO_WRITE_HIGH(GPC7);
                else
                        GPIO_WRITE_LOW(GPC7);

                printk("ADC0: %03d\r", value);
        }

        return 0;
}
