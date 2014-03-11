#include <arch/antares.h>

#include <lib/cerebellum/gpio.h>
#include <lib/printk.h>

#include <odetect/adc.h>
#include <odetect/sonar.h>

#include <util/delay.h>

#include <math.h>

ANTARES_INIT_LOW(buzzer)
{
        GPIO_INIT_OUT(GPB7);
}

ANTARES_INIT_HIGH(odetect_full)
{
        printk("All systems are ready\n");
        GPIO_WRITE_HIGH(GPB7);
        _delay_ms(50);
        GPIO_WRITE_LOW(GPB7);
}

uint8_t range(uint8_t ir, uint8_t sonar)
{
        if (abs(ir - sonar) <= 10) {
                return (ir + sonar) >> 1;
        } else {
                return 255; /* out of range or wrong sensor data */
        }
}

static uint8_t measures[CONFIG_ODETECT_NUM_SENSORS];

ANTARES_APP(odetect)
{
        adc_start();
        sonar_start();
        while (!adc_is_ready());
        while (!sonar_is_ready());

        uint8_t *ameasures = adc_get();
        uint16_t *smeasures = sonar_get();

        printk("Results: ");
        for (uint8_t i=0; i<CONFIG_ODETECT_NUM_SENSORS; i++) {
                //printk("%03u %05u", scale_adc(ameasures[i]), scale_sonar(smeasures[5]));
                printk("%03u", range(adc_scale(ameasures[i]), sonar_scale(smeasures[5])));
        }

        printk("\r");
}
