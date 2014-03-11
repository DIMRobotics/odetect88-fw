#include <avr/io.h>
#include <lib/cerebellum/gpio.h>
#include <avr/interrupt.h>
#include <lib/printk.h>

#include <odetect/adc.h>

#include <arch/antares.h>

static uint8_t measures[CONFIG_ODETECT_NUM_ADC];
static uint8_t muxes[CONFIG_ODETECT_NUM_ADC] = {
#ifdef CONFIG_ODETECT_ADC0
        0x00, /* ADC0 */
#endif
#ifdef CONFIG_ODETECT_ADC1
        0x01, /* ADC1 */
#endif
#ifdef CONFIG_ODETECT_ADC4
        0x04, /* ADC4 */
#endif
#ifdef CONFIG_ODETECT_ADC5
        0x05, /* ADC5 */
#endif
#ifdef CONFIG_ODETECT_ADC6
        0x06, /* ADC6 */
#endif
#ifdef CONFIG_ODETECT_ADC7
        0x07, /* ADC7 */
#endif
#ifdef CONFIG_ODETECT_ADC8
        0x20, /* ADC8 */
#endif
#ifdef CONFIG_ODETECT_ADC9
        0x21, /* ADC9 */
#endif
#ifdef CONFIG_ODETECT_ADC10
        0x22, /* ADC10 */
#endif
#ifdef CONFIG_ODETECT_ADC11
        0x23, /* ADC11 */
#endif
#ifdef CONFIG_ODETECT_ADC12
        0x24, /* ADC12 */
#endif
#ifdef CONFIG_ODETECT_ADC13
        0x25, /* ADC13 */
#endif
};

static uint8_t cnt = 0;

ANTARES_INIT_LOW(adc_init)
{
        ADMUX = (1<<ADLAR); /* ADC0 channel, AREF as reference */
        ADCSRA |= (1<<ADEN)|(1<<ADIE)|(1<<ADPS1);
        ADCSRB |= (1<<ADHSM); /* high-speed mode */
        #ifdef CONFIG_ODETECT_DEBUG
        printk("ADC initialization completed\n");
        #endif
}

void adc_start(void)
{
        cnt = 0;
        ADCSRA |= (1<<ADSC); /* start convertion */
}

uint8_t adc_is_ready(void)
{
        return cnt == 0;
}

uint8_t *adc_get(void)
{
        return measures;
}

uint8_t adc_scale(uint8_t x)
{
	if (x <= 20)
                return 160; /* max distance */
        else if (x <= 40)
                return 230 - (x << 2);
        else if (x <= 60)
                return 112 - x;
        else if (x <= 150)
                return 82 - (x >> 1);
        else
                return 0; /* wtf ?? */
}

static inline void adc_select_mux()
{
        ADMUX &= 0xE0; /* clear old MUX bits */
        ADMUX |= muxes[cnt] & 0x1F; /* set up lower bits */
        if (muxes[cnt] & 0x20) /* if MUX5 bit set */
                ADCSRB |= (1<<MUX5);
        else
                ADCSRB &= (1<<MUX5);
}

ISR(ADC_vect)
{
        measures[cnt++] = ADCH;

        if (cnt == CONFIG_ODETECT_NUM_ADC) {
                cnt = 0;
        } else {
                adc_select_mux();
                ADCSRA |= (1<<ADSC); /* start next convertion */
        }
}
