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
        GPIO_INIT_OUT(GPD4); /* led for channel 0 */
        GPIO_INIT_OUT(GPD6); /* led for channel 1 */
        GPIO_INIT_OUT(GPD7); /* led for channel 2 */
        GPIO_INIT_OUT(GPB6); /* led for channel 3 */
        GPIO_INIT_OUT(GPC6); /* led for channel 4 */
        GPIO_INIT_OUT(GPC7); /* led for channel 5 */
}

ANTARES_INIT_HIGH(odetect_full)
{
        printk("All systems are ready\n");
        GPIO_WRITE_HIGH(GPB7);
        _delay_ms(50);
        GPIO_WRITE_LOW(GPB7);
}

uint8_t range(uint8_t i, uint8_t ir, uint16_t sonar)
{
        if (i == 0)
                ir = adc_scale_ld(ir);
        else if (i == 1)
                ir = adc_scale_hd(ir);
        else if (i == 2)
                ir = adc_scale_md(ir);
        else if (i == 3)
                ir = adc_scale_ld(ir);
        else 
                ir = 0;

        uint8_t sn = sonar_scale(sonar);

        if (abs(ir - sn) <= 10) {
                return (ir + sn) >> 1;
        } else {
                return 255; /* out of range or wrong sensor data */
        }
}

static uint8_t measures[CONFIG_ODETECT_NUM_SENSORS];
static uint8_t limits[] = { 20, 40, 20, 20, 20, 20 };
static uint8_t sonar_table[] = { 0, 1, 5, 4 };

void led_on(uint8_t i)
{
        switch (i) {
                case 5:
                        GPIO_WRITE_HIGH(GPD4);
                        break;
                case 4:
                        GPIO_WRITE_HIGH(GPD6);
                        break;
                case 3:
                        GPIO_WRITE_HIGH(GPD7);
                        break;
                case 2:
                        GPIO_WRITE_HIGH(GPB6);
                        break;
                case 1:
                        GPIO_WRITE_HIGH(GPC6);
                        break;
                case 0:
                        GPIO_WRITE_HIGH(GPC7);
                        break;
        }
}

void led_off(uint8_t i)
{
        switch (i) {
                case 5:
                        GPIO_WRITE_LOW(GPD4);
                        break;
                case 4:
                        GPIO_WRITE_LOW(GPD6);
                        break;
                case 3:
                        GPIO_WRITE_LOW(GPD7);
                        break;
                case 2:
                        GPIO_WRITE_LOW(GPB6);
                        break;
                case 1:
                        GPIO_WRITE_LOW(GPC6);
                        break;
                case 0:
                        GPIO_WRITE_LOW(GPC7);
                        break;
        }
}

ANTARES_APP(odetect_process)
{
        adc_start();
        sonar_start();
        while (!adc_is_ready());
        while (!sonar_is_ready());

        uint8_t *ameasures = adc_get();
        uint16_t *smeasures = sonar_get();

        /* sensors 0,1 - high distance
         * 2,3 - medium distance,
         * 4,5 - low distance */

        uint8_t flag_on = 0;

        for (uint8_t i=0; i<CONFIG_ODETECT_NUM_SENSORS; i++) {
                measures[i] = range(i, ameasures[i], smeasures[sonar_table[i]]);

                if (measures[i] <= limits[i]) {
                        flag_on = 1;
                        led_on(i);
                } else {
                        led_off(i);
                }
        }

        if (flag_on)
                GPIO_WRITE_HIGH(GPB7);
        else
                GPIO_WRITE_LOW(GPB7);
        
        printk("Results: ");
        //for (uint8_t i=0; i<CONFIG_ODETECT_NUM_SENSORS; i++) {
                //printk("%03u ", measures[i]);
                printk("%03u %03u; ", adc_scale_hd(ameasures[1]), sonar_scale(smeasures[sonar_table[1]]));
        //}

        printk("\r");
}
